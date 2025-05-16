/*
 * File: lrucache.hpp
 *
 * Author: Alexander Ponomarev
 * Created on June 20, 2013, 5:09 PM
 *
 * Author: Guiorgy
 * Forked on October 23, 2024
 */

#pragma once

#include <unordered_map>
#include <type_traits>
#include <functional>
#include <optional>
#include <cassert>
#include <cstdint>
#include <cstddef>
#include <utility>
#include <limits>
#include <vector>

namespace guiorgy {
	template<typename key_t, typename value_t, const std::size_t max_size, const bool preallocate>
	class lru_cache;

	namespace detail {
		template<typename key_t, typename value_t, const std::size_t max_size>
		class lru_cache_storage_base;

		template <const std::size_t max_value>
		struct uint_fit final {
			static_assert(max_value >= 0ull, "std::size_t is less than 0?!");

			using type = std::conditional_t<
				max_value <= 255ull,
				std::uint8_t,
				std::conditional_t<
					max_value <= 65'535ull,
					std::uint16_t,
					std::conditional_t<
						max_value <= 4'294'967'295ull,
						std::uint32_t,
						std::uint64_t
					>
				>
			>;
		};

		template <const std::size_t max_value>
		using uint_fit_t = typename uint_fit<max_value>::type;

		template<typename T, typename index_t = std::size_t>
		class vector_set final {
			std::vector<T> set;
			index_t head = 0u;
			index_t tail = 0u;
			bool _empty = true;

			index_t next_index(const index_t index) const noexcept {
				return (std::size_t)index + 1u < set.size() ? index + 1u : 0u;
			}

		public:
			void reserve(const std::size_t capacity) {
				assert(capacity == 0 || capacity - 1 <= std::numeric_limits<index_t>::max());

				set.reserve(capacity);
			}

			bool empty() const noexcept {
				return _empty;
			}

			std::size_t size() const noexcept {
				if (_empty) {
					return 0u;
				} else {
					return tail <= head ? (std::size_t)head - tail + 1u : (std::size_t)head + 1u + (set.size() - tail);
				}
			}

			std::size_t capacity() const noexcept {
				return set.capacity();
			}

			void put(const T& value) {
				if (_empty) {
					_empty = false;

					if (set.size() == 0u) {
						set.push_back(value);
					} else {
						set[head] = value;
					}
				} else {
					index_t next_head = next_index(head);

					if (next_head == tail) {
						set.push_back(value);
					} else {
						head = next_head;
						set[head] = value;
					}
				}

				assert(set.size() == 0 || set.size() - 1 <= std::numeric_limits<index_t>::max());
			}

			void put(T&& value) {
				if (_empty) {
					_empty = false;

					if (set.size() == 0u) {
						set.push_back(std::move(value));
					} else {
						set[head] = value;
					}
				} else {
					index_t next_head = next_index(head);

					if (next_head == tail) {
						set.push_back(std::move(value));
					} else {
						head = next_head;
						set[head] = value;
					}
				}

				assert(set.size() == 0 || set.size() - 1 <= std::numeric_limits<index_t>::max());
			}

			T& peek() const {
				assert(!_empty);

				return set[tail];
			}

			T take() {
				assert(!_empty);

				T _front = set[tail];
				tail = next_index(tail);
				if (tail == head) _empty = true;
				return _front;
			}
		};

		template<typename T, const std::size_t max_size = std::numeric_limits<std::size_t>::max() - 1u>
		class vector_list final {
			using index_t = uint_fit_t<max_size>;
			static constexpr const index_t null_index = std::numeric_limits<index_t>::max();
			static_assert(max_size <= null_index, "null_index can not be less than max_size, since those are valid indeces");

			struct list_node final {
				T value;
				index_t prior;
				index_t next;
#ifndef NDEBUG
				bool removed;
#endif

				list_node(const T& value, const index_t prior, const index_t next) :
					value(value),
					prior(prior),
					next(next)
#ifndef NDEBUG
					, removed(false)
#endif
					{}

				list_node(T&& value, const index_t prior, const index_t next) :
					value(value),
					prior(prior),
					next(std::move(next))
#ifndef NDEBUG
					, removed(false)
#endif
					{}

				template<typename... ValueArgs>
				list_node(const index_t prior, const index_t next, ValueArgs&&... value_args) :
					value(std::forward<ValueArgs>(value_args)...),
					prior(prior),
					next(std::move(next))
#ifndef NDEBUG
					, removed(false)
#endif
					{}

				template<typename... ValueArgs>
				T& emplace_value(ValueArgs&&... value_args) {
					value.~T();
					::new (&value) T(std::forward<ValueArgs>(value_args)...);
					return value;
				}

				list_node() = default;
				~list_node() = default;
				list_node(const list_node&) = default;
				list_node(list_node&&) = default;
				list_node& operator=(list_node const&) = default;
				list_node& operator=(list_node &&) = default;
			};

			template<const bool constant, const bool reverse>
			class _iterator;

		public:
			using iterator = _iterator<false, false>;
			using const_iterator = _iterator<true, false>;
			using reverse_iterator = _iterator<false, true>;
			using const_reverse_iterator = _iterator<true, true>;

		private:
			std::vector<list_node> list;
			index_t head = null_index;
			index_t tail = null_index;
			vector_set<index_t, index_t> free_indices;

			list_node& get_node(const index_t at) {
				assert(at < list.size());
#ifndef NDEBUG
				assert(!list[at].removed);
#endif

				return list[at];
			}

			list_node& remove_node(const index_t at, const bool mark_removed = true) {
				assert(at < list.size());
#ifndef NDEBUG
				assert(!list[at].removed);
#endif

				list_node& _at = list[at];

				if (_at.prior != null_index && _at.next != null_index) {
					list[_at.prior].next = _at.next;
					list[_at.next].prior = _at.prior;
				} else if (_at.prior != null_index) {
					list[_at.prior].next = null_index;
				} else {
					list[_at.next].prior = null_index;
				}

				if (at == head) {
					head = _at.prior;
				} else if (at == tail) {
					tail = _at.next;
				}

				if (mark_removed) {
					free_indices.put(at);
#ifndef NDEBUG
					_at.removed = true;
#endif
				}

				return _at;
			}

			list_node& move_node(const index_t from, const index_t to, const bool before = true) {
				assert(head != null_index);
				assert(from < list.size());
				assert(to < list.size());
#ifndef NDEBUG
				assert(!list[from].removed);
				assert(!list[to].removed);
#endif

				if (from == to) return list[from];

				list_node& _from = remove_node(from, false);
				list_node& _to = list[to];

				if (before) {
					_from.prior = _to.prior;
					_from.next = to;
					_to.prior = from;
					if (to != tail) {
						assert(_to.prior != null_index);

						list[_to.prior].next = from;
					} else {
						assert(_to.prior == null_index);

						tail = from;
					}
				} else {
					_from.prior = to;
					_from.next = _to.next;
					_to.next = from;
					if (to != head) {
						assert(_to.next != null_index);

						list[_to.next].prior = from;
					} else {
						assert(_to.next == null_index);

						head = from;
					}
				}

				return _from;
			}

			list_node& move_node_to_front(const index_t from) {
				assert(head != null_index);
				assert(from < list.size());
#ifndef NDEBUG
				assert(!list[from].removed);
#endif

				if (from == head) return list[head];

				list_node& _from = remove_node(from, false);

				_from.prior = head;
				_from.next = null_index;
				list[head].next = from;
				head = from;

				return _from;
			}

			list_node& move_node_to_back(const index_t from) {
				assert(tail != null_index);
				assert(from < list.size());
#ifndef NDEBUG
				assert(!list[from].removed);
#endif

				if (from == tail) return list[tail];

				list_node& _from = remove_node(from, false);

				_from.prior = null_index;
				_from.next = tail;
				list[tail].prior = from;
				tail = from;

				return _from;
			}

		public:
			void reserve(const std::size_t capacity = max_size) {
				assert(capacity <= max_size);

				list.reserve(capacity);
				free_indices.reserve(capacity);
			}

			bool empty() const noexcept {
				return list.size() == free_indices.size();
			}

			std::size_t size() const noexcept {
				return list.size() - free_indices.size();
			}

			std::size_t capacity() const noexcept {
				return list.capacity();
			}

			void push_front(const T& value) {
				if (!free_indices.empty()) {
					index_t index = free_indices.take();
					list_node& node = list[index];

					node.value = value;
					node.prior = head;
					node.next = null_index;
#ifndef NDEBUG
					assert(node.removed);
					node.removed = false;
#endif

					if (head != null_index) list[head].next = index;
					head = index;
					if (tail == null_index) tail = head;
				} else {
					if (head != null_index) list[head].next = list.size();
					index_t prior = head;
					head = list.size();
					if (tail == null_index) tail = head;

					list.emplace_back(value, prior, null_index);
				}
			}

			void push_front(T&& value) {
				if (!free_indices.empty()) {
					index_t index = free_indices.take();
					list_node& node = list[index];

					node.value = std::move(value);
					node.prior = head;
					node.next = null_index;
#ifndef NDEBUG
					assert(node.removed);
					node.removed = false;
#endif

					if (head != null_index) list[head].next = index;
					head = index;
					if (tail == null_index) tail = head;
				} else {
					if (head != null_index) list[head].next = list.size();
					index_t prior = head;
					head = list.size();
					if (tail == null_index) tail = head;

					list.emplace_back(std::move(value), prior, null_index);
				}
			}

			template<typename... ValueArgs>
			T& emplace_front(ValueArgs&&... value_args) {
				if (!free_indices.empty()) {
					index_t index = free_indices.take();
					list_node& node = list[index];

					T& value = node.emplace_value(std::forward<ValueArgs>(value_args)...);
					node.prior = head;
					node.next = null_index;
#ifndef NDEBUG
					assert(node.removed);
					node.removed = false;
#endif

					if (head != null_index) list[head].next = index;
					head = index;
					if (tail == null_index) tail = head;

					return value;
				} else {
					if (head != null_index) list[head].next = list.size();
					index_t prior = head;
					head = list.size();
					if (tail == null_index) tail = head;

					return list.emplace_back(prior, null_index, std::forward<ValueArgs>(value_args)...).value;
				}
			}

			void push_back(const T& value) {
				if (!free_indices.empty()) {
					index_t index = free_indices.take();
					list_node& node = list[index];

					node.value = value;
					node.prior = null_index;
					node.next = tail;
#ifndef NDEBUG
					assert(node.removed);
					node.removed = false;
#endif

					if (tail != null_index) list[tail].prior = index;
					tail = index;
					if (head == null_index) head = tail;
				} else {
					if (tail != null_index) list[tail].prior = list.size();
					index_t next = tail;
					tail = list.size();
					if (head == null_index) head = tail;

					list.emplace_back(value, null_index, next);
				}
			}

			void push_back(T&& value) {
				if (!free_indices.empty()) {
					index_t index = free_indices.take();
					list_node& node = list[index];

					node.value = std::move(value);
					node.prior = null_index;
					node.next = tail;
#ifndef NDEBUG
					assert(node.removed);
					node.removed = false;
#endif

					if (tail != null_index) list[tail].prior = index;
					tail = index;
					if (head == null_index) head = tail;
				} else {
					if (tail != null_index) list[tail].prior = list.size();
					index_t next = tail;
					tail = list.size();
					if (head == null_index) head = tail;

					list.emplace_back(std::move(value), null_index, next);
				}
			}

			template<typename... ValueArgs>
			T& emplace_back(ValueArgs&&... value_args) {
				if (!free_indices.empty()) {
					index_t index = free_indices.take();
					list_node& node = list[index];

					T& value = node.emplace_value(std::forward<ValueArgs>(value_args)...);
					node.prior = null_index;
					node.next = tail;
#ifndef NDEBUG
					assert(node.removed);
					node.removed = false;
#endif

					if (tail != null_index) list[tail].prior = index;
					tail = index;
					if (head == null_index) head = tail;

					return value;
				} else {
					if (tail != null_index) list[tail].prior = list.size();
					index_t next = tail;
					tail = list.size();
					if (head == null_index) head = tail;

					return list.emplace_back(null_index, next, std::forward<ValueArgs>(value_args)...).value;
				}
			}

			T& front() {
				assert(head != null_index);
				assert(size() != 0u);

				return list[head].value;
			}

			T& pop_front_ref() {
				assert(head != null_index);
				assert(size() != 0u);

				T& _front = list[head].value;
#ifndef NDEBUG
				assert(!list[head].removed);
				list[head].removed = true;
#endif
				free_indices.put(head);
				head = _front.prior;
				if (head == null_index) tail = null_index;
				else list[head].next = null_index;
				return _front;
			}

			T pop_front() {
				return pop_front_ref();
			}

			T& back() {
				assert(tail != null_index);
				assert(size() != 0u);

				return list[tail].value;
			}

			T& pop_back_ref() {
				assert(tail != null_index);
				assert(size() != 0u);

				T& _back = list[tail].value;
#ifndef NDEBUG
				assert(!list[tail].removed);
				list[tail].removed = true;
#endif
				free_indices.put(tail);
				tail = _back.next;
				if (tail == null_index) head = null_index;
				else list[tail].next = null_index;
				return _back;
			}

			T pop_back() {
				return pop_back_ref();
			}

			template<const bool reverse>
			T& move_to_front(const _iterator<false, reverse> it) {
				return move_node_to_front(it.forward_index()).value;
			}

			template<const bool reverse>
			T& move_to_back(const _iterator<false, reverse> it) {
				return move_node_to_back(it.forward_index()).value;
			}

			template<const bool reverse>
			T& erase(const _iterator<false, reverse> it) {
				remove_node(it.forward_index()).value;
			}

			void clear() {
				free_indices.reserve(list.size());
				for (index_t index = tail; index != null_index; index = list[index].next) {
					free_indices.put(index);
#ifndef NDEBUG
					assert(!list[index].removed);
					list[index].removed = true;
#endif
				}
				head = null_index;
				tail = null_index;
			}

			iterator begin() const noexcept {
				return iterator(list, head);
			}

			iterator end() const noexcept {
				return iterator(list, null_index);
			}

			const_iterator cbegin() const noexcept {
				return const_iterator(list, head);
			}

			const_iterator cend() const noexcept {
				return const_iterator(list, null_index);
			}

			reverse_iterator rbegin() const noexcept {
				return reverse_iterator(list, null_index);
			}

			reverse_iterator rend() const noexcept {
				return reverse_iterator(list, head);
			}

			const_reverse_iterator crbegin() const noexcept {
				return const_reverse_iterator(list, null_index);
			}

			const_reverse_iterator crend() const noexcept {
				return const_reverse_iterator(list, head);
			}

		private:
			template<const bool constant, const bool reverse>
			class _iterator final {
			public:
				using iterator_category = std::bidirectional_iterator_tag;
				using difference_type = std::ptrdiff_t;
				using value_type = std::remove_const_t<T>;
				using pointer = std::conditional_t<constant, const T*, T*>;
				using reference = std::conditional_t<constant, const T&, T&>;

			private:
				const vector_list<T, max_size>& list;
				index_t current_index;

				_iterator(const vector_list<T, max_size>& list, const index_t index) : list(list), current_index(index) {}

			public:
				template<const bool this_constant = constant, typename = typename std::enable_if_t<this_constant>>
				_iterator(const _iterator<false, reverse>& it) : list(it.list), current_index(it.current_index) {}

				template<const bool this_constant = constant, typename = typename std::enable_if_t<!this_constant>>
				_iterator(const _iterator<true, reverse>& it) = delete;

				template<const bool this_reverse = reverse, typename = typename std::enable_if_t<this_reverse>>
				_iterator(const _iterator<constant, false>& it) : list(it.list), current_index(current_index != null_index ? list[current_index].prior : null_index) {}

				template<const bool this_reverse = reverse, typename = typename std::enable_if_t<!this_reverse>>
				_iterator(const _iterator<constant, true>& it) : list(it.list), current_index(current_index == null_index ? list.tail : list[current_index].next) {}

				_iterator() = delete;
				~_iterator() = default;
				_iterator(const _iterator&) = default;
				_iterator(_iterator&&) = default;
				_iterator& operator=(_iterator const&) = default;
				_iterator& operator=(_iterator &&) = default;

			private:
				index_t forward_index() const noexcept {
					if constexpr (!reverse) {
						return current_index;
					} else {
						return current_index == null_index ? list.tail : list[current_index].next;
					}
				}

			public:
				reference operator*() const {
					return list[forward_index()].value;
				}

				pointer operator->() const {
					return &(list[forward_index()].value);
				}

				_iterator& operator++() {
					if constexpr (!reverse) {
						if (current_index != null_index) current_index = list[current_index].prior;
					} else {
						if (current_index != list.head) current_index = current_index == null_index ? list.tail : list[current_index].next;
					}

					return *this;
				}

				_iterator operator++(int) {
					_iterator temp = *this;
					++(*this);
					return temp;
				}

				_iterator& operator--() {
					if constexpr (!reverse) {
						if (current_index != list.head) current_index = current_index == null_index ? list.tail : list[current_index].next;
					} else {
						if (current_index != null_index) current_index = list[current_index].prior;
					}

					return *this;
				}

				_iterator operator--(int) {
					_iterator temp = *this;
					--(*this);
					return temp;
				}

				bool operator==(const _iterator& other) const noexcept {
					return current_index == other.current_index;
				}

				bool operator!=(const _iterator& other) const noexcept {
					return !(*this == other);
				}

			private:
				friend class vector_list<T, max_size>;
			};

		private:
			template<typename kt, typename vt, const std::size_t ms>
			friend class lru_cache_storage_base;

			template<typename kt, typename vt, const std::size_t ms, const bool p>
			friend class guiorgy::lru_cache;

			T& _move_value_at_to_front(const index_t position) {
				return move_node_to_front(position).value;
			}

			T& _move_value_at_to_back(const index_t position) {
				return move_node_to_back(position).value;
			}

			T& _get_value_at(const index_t position) {
				return get_node(position).value;
			}

			T& _erase_value_at(const index_t position) {
				return remove_node(position);
			}

			T& _move_last_value_to_front() {
				return _move_value_at_to_front(tail);
			}

			T& _move_first_value_to_back() {
				return _move_value_at_to_back(head);
			}

			index_t _first_value_index() const noexcept {
				assert(head != null_index);

				return head;
			}

			index_t _last_value_index() const noexcept {
				assert(tail != null_index);

				return tail;
			}
		};

		template<typename key_t, typename value_t, const std::size_t max_size>
		class lru_cache_storage_base {
		protected:
			typedef typename std::pair<key_t, value_t> key_value_pair_t;
			typedef typename vector_list<key_value_pair_t, max_size>::index_t list_index_t;

		public:
			typedef typename vector_list<key_value_pair_t, max_size>::const_iterator const_iterator;
			typedef typename vector_list<key_value_pair_t, max_size>::const_reverse_iterator const_reverse_iterator;

		protected:
			vector_list<key_value_pair_t, max_size> _cache_items_list;
			std::unordered_map<key_t, list_index_t> _cache_items_map;
		};

		template<typename key_t, typename value_t, const std::size_t max_size, const bool preallocate = false>
		class lru_cache_base : protected lru_cache_storage_base<key_t, value_t, max_size> {
		public:
			lru_cache_base() = default;
			~lru_cache_base() = default;
			lru_cache_base(const lru_cache_base&) = default;
			lru_cache_base(lru_cache_base&&) = default;
			lru_cache_base& operator=(lru_cache_base const&) = default;
			lru_cache_base& operator=(lru_cache_base &&) = default;
		};

		template<typename key_t, typename value_t, const std::size_t max_size>
		class lru_cache_base<key_t, value_t, max_size, true> : protected lru_cache_storage_base<key_t, value_t, max_size> {
		public:
			typedef typename lru_cache_storage_base<key_t, value_t, max_size>::key_value_pair_t key_value_pair_t;

			lru_cache_base() {
				this->_cache_items_list.reserve(max_size);
				this->_cache_items_map.reserve(max_size);
			}
			~lru_cache_base() = default;
			lru_cache_base(const lru_cache_base&) = default;
			lru_cache_base(lru_cache_base&&) = default;
			lru_cache_base& operator=(lru_cache_base const&) = default;
			lru_cache_base& operator=(lru_cache_base &&) = default;
		};
	}

	template<typename key_t, typename value_t, const std::size_t max_size, const bool preallocate = false>
	class lru_cache final : private detail::lru_cache_base<key_t, value_t, max_size, preallocate> {
		static_assert(max_size != 0u, "max_size can not be 0");

		typedef typename detail::lru_cache_storage_base<key_t, value_t, max_size>::key_value_pair_t key_value_pair_t;
		typedef typename detail::lru_cache_storage_base<key_t, value_t, max_size>::list_index_t list_index_t;

	public:
		typedef typename detail::lru_cache_storage_base<key_t, value_t, max_size>::const_iterator const_iterator;
		typedef typename detail::lru_cache_storage_base<key_t, value_t, max_size>::const_reverse_iterator const_reverse_iterator;

		void put(const key_t& key, const value_t& value) {
			auto it = this->_cache_items_map.find(key);
			if (it != this->_cache_items_map.end()) {
				this->_cache_items_list._get_value_at(it->second).second = value;
				this->_cache_items_list._move_value_at_to_front(it->second);
			} else {
				if (this->_cache_items_map.size() < max_size) {
					this->_cache_items_list.emplace_front(key, value);
				} else {
					key_value_pair_t& last = this->_cache_items_list.back();
					this->_cache_items_map.erase(last.first);
					last.first = key;
					last.second = value;
					this->_cache_items_list._move_last_value_to_front();
				}

				this->_cache_items_map[key] = this->_cache_items_list._first_value_index();
			}
		}

		void put(const key_t& key, value_t&& value) {
			auto it = this->_cache_items_map.find(key);
			if (it != this->_cache_items_map.end()) {
				this->_cache_items_list._get_value_at(it->second).second = std::move(value);
				this->_cache_items_list._move_value_at_to_front(it->second);
			} else {
				if (this->_cache_items_map.size() < max_size) {
					this->_cache_items_list.emplace_front(key, std::move(value));
				} else {
					key_value_pair_t& last = this->_cache_items_list.back();
					this->_cache_items_map.erase(last.first);
					last.first = key;
					last.second = std::move(value);
					this->_cache_items_list._move_last_value_to_front();
				}

				this->_cache_items_map[key] = this->_cache_items_list._first_value_index();
			}
		}

		template<typename... ValueArgs>
		const value_t& emplace(const key_t& key, ValueArgs&&... value_args) {
			auto it = this->_cache_items_map.find(key);
			if (it != this->_cache_items_map.end()) {
				value_t& value = this->_cache_items_list._get_value_at(it->second).second;
				value.~value_t();
				::new (&value) value_t(std::forward<ValueArgs>(value_args)...);
				this->_cache_items_list._move_value_at_to_front(it->second);

				return value;
			} else {
				value_t* value;

				if (this->_cache_items_map.size() < max_size) {
					value = &(this->_cache_items_list.emplace_front(key, std::forward<ValueArgs>(value_args)...));
				} else {
					key_value_pair_t& last = this->_cache_items_list.back();
					this->_cache_items_map.erase(last.first);
					last.first = key;
					last.second.~value_t();
					::new (&last.second) value_t(std::forward<ValueArgs>(value_args)...);
					this->_cache_items_list._move_last_value_to_front();

					value = &last.second;
				}

				this->_cache_items_map[key] = this->_cache_items_list._first_value_index();

				return *value;
			}
		}

		const std::optional<value_t> get(const key_t& key) {
			auto it = this->_cache_items_map.find(key);

			if (it == this->_cache_items_map.end()) {
				return std::nullopt;
			} else {
				return this->_cache_items_list._move_value_at_to_front(it->second).second;
			}
		}

		const std::optional<std::reference_wrapper<const value_t>> get_ref(const key_t& key) {
			auto it = this->_cache_items_map.find(key);

			if (it == this->_cache_items_map.end()) {
				return std::nullopt;
			} else {
				return std::make_optional(std::cref(this->_cache_items_list._move_value_at_to_front(it->second).second));
			}
		}

		bool try_get(const key_t& key, value_t& value_out) {
			auto it = this->_cache_items_map.find(key);

			if (it == this->_cache_items_map.end()) {
				return false;
			} else {
				value_out = this->_cache_items_list._move_value_at_to_front(it->second).second;
				return true;
			}
		}

		bool try_get_ref(const key_t& key, const value_t*& value_out) {
			auto it = this->_cache_items_map.find(key);

			if (it == this->_cache_items_map.end()) {
				value_out = nullptr;
				return false;
			} else {
				value_out = &(this->_cache_items_list._move_value_at_to_front(it->second).second);
				return true;
			}
		}

		std::optional<value_t> remove(const key_t& key) {
			auto it = this->_cache_items_map.find(key);

			if (it != this->_cache_items_map.end()) {
				value_t& value = this->_cache_items_list._erase_value_at(it->second).second;
				this->_cache_items_map.erase(it);
				return value;
			} else {
				return std::nullopt;
			}
		}

		std::optional<std::reference_wrapper<value_t>> remove_ref(const key_t& key) {
			auto it = this->_cache_items_map.find(key);

			if (it != this->_cache_items_map.end()) {
				value_t& value = this->_cache_items_list._erase_value_at(it->second).second;
				this->_cache_items_map.erase(it);
				return std::make_optional(std::ref(value));
			} else {
				return std::nullopt;
			}
		}

		bool try_remove(const key_t& key, value_t& value_out) {
			map_iterator it = this->_cache_items_map.find(key);

			if (it != this->_cache_items_map.end()) {
				value_out = this->_cache_items_list._erase_value_at(it->second).second;
				this->_cache_items_map.erase(it);
				return true;
			} else {
				return false;
			}
		}

		bool try_remove_ref(const key_t& key, const value_t*& value_out) {
			map_iterator it = this->_cache_items_map.find(key);

			if (it != this->_cache_items_map.end()) {
				value_out = &(this->_cache_items_list._erase_value_at(it->second).second);
				this->_cache_items_map.erase(it);
				return true;
			} else {
				return false;
			}
		}

		bool exists(const key_t& key) const {
			return this->_cache_items_map.find(key) != this->_cache_items_map.end();
		}

		std::size_t size() const noexcept {
			return this->_cache_items_map.size();
		}

		void clear() noexcept {
			this->_cache_items_map.clear();
			this->_cache_items_list.clear();
		}

		void reserve() {
			this->_cache_items_map.reserve(max_size);
			this->_cache_items_list.reserve(max_size);
		}

		const_iterator cbegin() const noexcept {
			return this->_cache_items_list.cbegin();
		}

		const_iterator begin() const noexcept {
			return cbegin();
		}

		const_iterator cend() const noexcept {
			return this->_cache_items_list.cend();
		}

		const_iterator end() const noexcept {
			return cend();
		}

		const_reverse_iterator crbegin() const noexcept {
			return this->_cache_items_list.crbegin();
		}

		const_iterator rbegin() const noexcept {
			return crbegin();
		}

		const_reverse_iterator crend() const noexcept {
			return this->_cache_items_list.crend();
		}

		const_iterator rend() const noexcept {
			return crend();
		}
	};
}
