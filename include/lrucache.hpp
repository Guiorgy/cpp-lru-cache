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

// Define possible implementation and store old definitions to restore later in case of a conflict
	// STL std::unordered_map
	#ifdef STL_UNORDERED_MAP
		#define GUIORGY_LRU_CACHE_STL_UNORDERED_MAP_BEFORE STL_UNORDERED_MAP
	#endif
	#define STL_UNORDERED_MAP 10

	// Abseil absl::flat_hash_map
	#ifdef ABSEIL_FLAT_HASH_MAP
		#define GUIORGY_LRU_CACHE_ABSEIL_FLAT_HASH_MAP_BEFORE ABSEIL_FLAT_HASH_MAP
	#endif
	#define ABSEIL_FLAT_HASH_MAP 20

// Set the default implementation
#ifndef LRU_CACHE_HASH_MAP_IMPLEMENTATION
	#define LRU_CACHE_HASH_MAP_IMPLEMENTATION STL_UNORDERED_MAP
#endif

// Use the correct headers for the selected implementation
#if LRU_CACHE_HASH_MAP_IMPLEMENTATION == STL_UNORDERED_MAP
	#include <unordered_map>

	#pragma message("Using std::unordered_map as the hashmap for the LRU cache")
#elif LRU_CACHE_HASH_MAP_IMPLEMENTATION == ABSEIL_FLAT_HASH_MAP
	#include "absl/container/flat_hash_map.h"

	#pragma message("Using absl::flat_hash_map as the hashmap for the LRU cache")
#else
	#define VALUE_TO_STRING(x) #x
	#define VALUE(x) VALUE_TO_STRING(x)

	#pragma message("LRU_CACHE_HASH_MAP_IMPLEMENTATION is set to " VALUE(LRU_CACHE_HASH_MAP_IMPLEMENTATION))
	#pragma message("Possible valiues are STL_UNORDERED_MAP(std::unordered_map), ABSEIL_FLAT_HASH_MAP(absl::flat_hash_map)")
	#error "Unexpected value of LRU_CACHE_HASH_MAP_IMPLEMENTATION"
#endif

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
	// Forward declaration of lru_cache.
	template<typename key_t, typename value_t, const std::size_t max_size, const bool preallocate>
	class lru_cache;

	namespace detail {
		// Forward declaration of lru_cache_storage_base.
		template<typename key_t, typename value_t, const std::size_t max_size>
		class lru_cache_storage_base;

		// Forward declaration of vector_list.
		template<typename T, const std::size_t max_size>
		class vector_list;

		// Determine the smallest unsigned integer type that can fit the specified value.
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

		// Helper for uint_fit.
		template <const std::size_t max_value>
		using uint_fit_t = typename uint_fit<max_value>::type;

		// A container where you can store elements and then take them out in an unspecified order.
		// Remarks:
		//   - The size of the container must not exceed the maximum value representable by index_t plus one.
		//   - The size limitation is not enforced within the container, the user must ensure that this condition is not violated.
		template<typename T, typename index_t = std::size_t>
		class vector_set final {
			std::vector<T> set;
			index_t head = 0u;
			index_t tail = 0u;
			bool _empty = true;

			// Returns the index of the next element to take.
			index_t next_index(const index_t index) const noexcept {
				return (std::size_t)index + 1u < set.size() ? index + 1u : 0u;
			}

		public:
			// Increases the capacity of the set (the total number of elements that the set can hold without requiring a reallocation) to a value that's greater or equal to capacity.
			void reserve(const std::size_t capacity) {
				assert(capacity == 0u || capacity - 1u <= std::numeric_limits<index_t>::max());

				set.reserve(capacity);
			}

			// Checks whether the set is empty.
			bool empty() const noexcept {
				return _empty;
			}

			// Returns the number of elements in the set.
			std::size_t size() const noexcept {
				if (_empty) {
					return 0u;
				} else {
					return tail <= head ? (std::size_t)head - tail + 1u : (std::size_t)head + 1u + (set.size() - tail);
				}
			}

			// Returns the number of elements that the set has currently allocated space for.
			std::size_t capacity() const noexcept {
				return set.capacity();
			}

			// Puts the given element into the set.
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

				assert(set.size() == 0u || set.size() - 1u <= std::numeric_limits<index_t>::max());
			}

			// Puts the given element into the set.
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

				assert(set.size() == 0u || set.size() - 1u <= std::numeric_limits<index_t>::max());
			}

			// Returns a reference to the next element in the set.
			T& peek() const {
				assert(!_empty);

				return set[tail];
			}

			// Removes the next element in the set and returns it.
			T take() {
				assert(!_empty);

				T _front = set[tail];
				tail = next_index(tail);
				if (tail == head) _empty = true;
				return _front;
			}

			// Erases all elements from the set. After this call, size() returns zero.
			// References referring to contained elements are not invalidated, since the elements are deleted lazily.
			void clear() {
				_empty = true;
				head = 0u;
				tail = 0u;
			}

		private:
			// Declare vector_list as a friend to allow access to _clear_and_fill_range.
			template<typename t, const std::size_t ms>
			friend class vector_list;

			// Equivalent to calling clear() and then putting values 0 to count into the set.
			// This assumes T is an unsigned integer.
			// Only meant to be used by vector_list.
			void _clear_and_fill_range(const std::size_t count) {
				if (count == 0) {
					clear();
					return;
				}

				const std::size_t _size = set.size();
				if (count <= _size) {
					for (std::size_t i = 0u; i < count; ++i) {
						set[i] = i;
					}
				} else {
					reserve(count);

					for (std::size_t i = 0u; i < _size; ++i) {
						set[i] = i;
					}
					for (std::size_t i = _size; i < count; ++i) {
						set.push_back(i);
					}
				}

				tail = 0u;
				head = count - 1u;
			}
		};

		// A doubly linked list backed by a sdt::vector, whos nodes refer to each other by indeces instead of pointers.
		// This allows their reallocation, keeping them close to each other in memory as the list grows.
		// It also allows the preallocation of the necessay memory to hold the desired number of elements without additional allocations.
		// Remarks:
		//   - The size of the list must not exceed max_size.
		//   - The size limitation is not enforced within the container, the user must ensure that this condition is not violated.
		//   - The removed elements are not deleted immediately, instead they are marked as free and replaced when new elements are pushed into the container.
		template<typename T, const std::size_t max_size = std::numeric_limits<std::size_t>::max() - 1u>
		class vector_list final {
			using index_t = uint_fit_t<max_size>;
			static constexpr const index_t null_index = std::numeric_limits<index_t>::max();
			static_assert(max_size <= null_index, "null_index can not be less than max_size, since those are valid indeces");

			// The internal node structure of the list.
			struct list_node final {
				T value;
				index_t prior;
				index_t next;
#ifndef NDEBUG
				bool removed; // For debug assertions to check the correctness of the list.
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

			// Forward declaration of _iterator.
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

			// Returns a reference to the node at the specified location.
			list_node& get_node(const index_t at) {
				assert(at < list.size());
#ifndef NDEBUG
				assert(!list[at].removed);
#endif

				return list[at];
			}

			// Removes the node at the specified location and returns a reference to the removed node.
			// References to the removed node are not invalidated, since the node is just marked as removed and added to the free_indices set.
			// Iterators to the removed node are invalidated. Other iterators are not affected.
			// If mark_removed is false, then the removed node is not marked as removed and is not added to the free_indices set.
			// Set mark_removed to false only if the node is to be reincluded back into the list at a different location immediately after the removal.
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

			// Moves the node at the specified location to before/after another node and returns a reference to the moved node.
			// References to the moved/destination node are not invalidated, since only the prior and next members of the node are updated.
			// Iterators to the moved node are invalidated. Other iterators, including the iterators to the node at the movement destination, are not affected.
			// If before is true, then the node is moved before the node at the destination, otherwise, the node is moved after the destination.
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

			// Moves the node at the specified location to the front of the list and returns a reference to the moved node.
			// References to the moved/head node are not invalidated, since only the prior and next members of the node are updated.
			// Iterators to the moved node are invalidated. Other iterators, including the iterators to the head node, are not affected.
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

			// Moves the node at the specified location to the back of the list and returns a reference to the moved node.
			// References to the moved/tail node are not invalidated, since only the prior and next members of the node are updated.
			// Iterators to the moved node are invalidated. Other iterators, including the iterators to the tail node, are not affected.
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
			// Increases the capacity of the list (the total number of elements that the list can hold without requiring a reallocation) to a value that's greater or equal to capacity.
			void reserve(const std::size_t capacity = max_size) {
				assert(capacity <= max_size);

				list.reserve(capacity);
				free_indices.reserve(capacity);
			}

			// Checks whether the list is empty.
			bool empty() const noexcept {
				return list.size() == free_indices.size();
			}

			// Returns the number of elements in the list.
			std::size_t size() const noexcept {
				return list.size() - free_indices.size();
			}

			// Returns the number of elements that the list has currently allocated space for.
			std::size_t capacity() const noexcept {
				return list.capacity();
			}

			// Prepends a copy of value to the beginning of the list.
			// If after the operation the new size() is greater than old capacity() a reallocation takes place, in which case all references are invalidated.
			// No iterators are invalidated.
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

			// Prepends value to the beginning of the list.
			// If after the operation the new size() is greater than old capacity() a reallocation takes place, in which case all references are invalidated.
			// No iterators are invalidated.
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

			// Prepends a new element to the beginning of the list.
			// The element is constructed in-place.
			// The arguments args... are forwarded to the constructor.
			// If after the operation the new size() is greater than old capacity() a reallocation takes place, in which case all references are invalidated.
			// No iterators are invalidated.
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

			// Appends a copy of value to the end of the list.
			// If after the operation the new size() is greater than old capacity() a reallocation takes place, in which case all references are invalidated.
			// No iterators are invalidated.
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

			// Appends value to the end of the list.
			// If after the operation the new size() is greater than old capacity() a reallocation takes place, in which case all references are invalidated.
			// No iterators are invalidated.
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

			// Appends a new element to the end of the list.
			// The element is constructed in-place.
			// The arguments args... are forwarded to the constructor.
			// If after the operation the new size() is greater than old capacity() a reallocation takes place, in which case all references are invalidated.
			// No iterators are invalidated.
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

			// Returns a reference to the first element in the list.
			T& front() {
				assert(head != null_index);
				assert(size() != 0u);

				return list[head].value;
			}

			// Removes the first element of the list and returns a reference to the removed element.
			// References to the removed element are not invalidated, since the element is deleted lazily.
			// Iterators to the removed element are invalidated. Other iterators are not affected.
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

			// Removes the first element of the list and returns a copy of ithe removed element.
			// References to the removed element are not invalidated, since the element is deleted lazily.
			// Iterators to the removed element are invalidated. Other iterators are not affected.
			T pop_front() {
				return pop_front_ref();
			}

			// Returns a reference to the last element in the list.
			T& back() {
				assert(tail != null_index);
				assert(size() != 0u);

				return list[tail].value;
			}

			// Removes the last element of the list and returns a reference to the removed element.
			// References to the removed element are not invalidated, since the element is deleted lazily.
			// Iterators to the removed element are invalidated. Other iterators are not affected.
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

			// Removes the last element of the list and returns a copy of ithe removed element.
			// References to the removed element are not invalidated, since the element is deleted lazily.
			// Iterators to the removed element are invalidated. Other iterators are not affected.
			T pop_back() {
				return pop_back_ref();
			}

			// Moves the element at the specified location to the front of the list and returns a reference to the moved element.
			// References to the moved/front element are not invalidated.
			// Iterators to the moved element are invalidated. Other iterators, including the iterators to the front element, are not affected.
			template<const bool reverse>
			T& move_to_front(const _iterator<false, reverse> it) {
				return move_node_to_front(it.forward_index()).value;
			}

			// Moves the element at the specified location to the back of the list and returns a reference to the moved element.
			// References to the moved/back element are not invalidated.
			// Iterators to the moved element are invalidated. Other iterators, including the iterators to the back element, are not affected.
			template<const bool reverse>
			T& move_to_back(const _iterator<false, reverse> it) {
				return move_node_to_back(it.forward_index()).value;
			}

			// Removes the element at the specified location and returns a reference to the removed element.
			// References to the removed element are not invalidated, since the element is deleted lazily.
			// Iterators to the removed element are invalidated. Other iterators are not affected.
			template<const bool reverse>
			T& erase(const _iterator<false, reverse> it) {
				remove_node(it.forward_index()).value;
			}

			// Erases all elements from the list. After this call, size() returns zero.
			// References referring to contained elements are not invalidated, since the elements are deleted lazily.
			// Invalidates any iterators referring to contained elements.
			void clear() {
#ifndef NDEBUG
				for (index_t index = head; index != null_index; index = list[index].prior) {
					assert(!list[index].removed);
					list[index].removed = true;
				}
				for (const list_node& node : list) {
					assert(node.removed);
				}
#endif

				free_indices._clear_and_fill_range(list.size());
				head = null_index;
				tail = null_index;
			}

			// Returns an iterator to the front element of the list.
			iterator begin() const noexcept {
				return iterator(list, head);
			}

			// Returns an iterator to an invalid element of the list.
			// This returned iterator only acts as a sentinel. It is not guaranteed to be dereferenceable.
			iterator end() const noexcept {
				return iterator(list, null_index);
			}

			// Returns a const iterator to the front element of the list.
			const_iterator cbegin() const noexcept {
				return const_iterator(list, head);
			}

			// Returns a const iterator to an invalid element of the list.
			// This returned iterator only acts as a sentinel. It is not guaranteed to be dereferenceable.
			const_iterator cend() const noexcept {
				return const_iterator(list, null_index);
			}

			// Returns a reverse iterator to the front element of the reversed list.
			// It corresponds to the back element of the non-reversed list.
			reverse_iterator rbegin() const noexcept {
				return reverse_iterator(list, null_index);
			}

			// Returns a reverse iterator to an invalid element of the list.
			// It corresponds to the element preceding the first element of the non-reversed list.
			// This returned iterator only acts as a sentinel. It is not guaranteed to be dereferenceable.
			reverse_iterator rend() const noexcept {
				return reverse_iterator(list, head);
			}

			// Returns a const reverse iterator to the front element of the reversed list.
			// It corresponds to the back element of the non-reversed list.
			const_reverse_iterator crbegin() const noexcept {
				return const_reverse_iterator(list, null_index);
			}

			// Returns a const reverse iterator to an invalid element of the list.
			// It corresponds to the element preceding the first element of the non-reversed list.
			// This returned iterator only acts as a sentinel. It is not guaranteed to be dereferenceable.
			const_reverse_iterator crend() const noexcept {
				return const_reverse_iterator(list, head);
			}

		private:
			// Template for iterator, const_iterator, reverse_iterator and const_reverse_iterator iterators of vector_list.
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
				// Copy constructor from (reverse_)iterator to const_(reverse_)iterator.
				template<const bool this_constant = constant, typename = typename std::enable_if_t<this_constant>>
				_iterator(const _iterator<false, reverse>& it) : list(it.list), current_index(it.current_index) {}

				// Copy constructor from const_(reverse_)iterator to (reverse_)iterator.
				// Explicitly deleted, since this would violate the const constraint.
				template<const bool this_constant = constant, typename = typename std::enable_if_t<!this_constant>>
				_iterator(const _iterator<true, reverse>& it) = delete;

				// Copy constructor from (const_)iterator to (const_)reverse_iterator.
				template<const bool this_reverse = reverse, typename = typename std::enable_if_t<this_reverse>>
				_iterator(const _iterator<constant, false>& it) : list(it.list), current_index(current_index != null_index ? list[current_index].prior : null_index) {}

				// Copy constructor from (const_)reverse_iterator to (const_)iterator.
				template<const bool this_reverse = reverse, typename = typename std::enable_if_t<!this_reverse>>
				_iterator(const _iterator<constant, true>& it) : list(it.list), current_index(current_index == null_index ? list.tail : list[current_index].next) {}

				_iterator() = delete;
				~_iterator() = default;
				_iterator(const _iterator&) = default;
				_iterator(_iterator&&) = default;
				_iterator& operator=(_iterator const&) = default;
				_iterator& operator=(_iterator &&) = default;

			private:
				// Normalizes the index to the one used in forward iteration.
				// In other words, if the current iterator is a (const_)reverse_iterator this returns the index to the list node this iterator actually refers to.
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
			// Declare lru_cache_storage_base as a friend to allow access to index_t.
			template<typename kt, typename vt, const std::size_t ms>
			friend class lru_cache_storage_base;

			// Declare lru_cache as a friend to give access to the dangerous member functions below.
			template<typename kt, typename vt, const std::size_t ms, const bool p>
			friend class guiorgy::lru_cache;

			// The functions below expose and operate internal indeces instead of using public iterators.
			// They are only indtended to be used by lru_cache to avoid the overhead of using iterators.
			// Use them with caution.

			// See move_node_to_front for details.
			T& _move_value_at_to_front(const index_t position) {
				return move_node_to_front(position).value;
			}

			// See move_node_to_back for details.
			T& _move_value_at_to_back(const index_t position) {
				return move_node_to_back(position).value;
			}

			// See get_node for details.
			T& _get_value_at(const index_t position) {
				return get_node(position).value;
			}

			// See remove_node for details.
			T& _erase_value_at(const index_t position) {
				return remove_node(position).value;
			}

			// See move_node_to_front for details.
			T& _move_last_value_to_front() {
				return _move_value_at_to_front(tail);
			}

			// See move_node_to_back for details.
			T& _move_first_value_to_back() {
				return _move_value_at_to_back(head);
			}

			// Returns the index to the first element in the list, or null_index if the list is empty.
			index_t _first_value_index() const noexcept {
				assert(head != null_index);

				return head;
			}

			// Returns the index to the last element in the list, or null_index if the list is empty.
			index_t _last_value_index() const noexcept {
				assert(tail != null_index);

				return tail;
			}
		};

		// lru_cache_storage_base and lru_cache_base(<key_t, value_t, max_size>) are used
		// to allow the conditional declaration of a default or custom constructor in C++ 17.
		// For more details see: https://devblogs.microsoft.com/cppblog/conditionally-trivial-special-member-functions/
		// Although, turns out std::vector is not trivially (default) constructible anyway, so this is actually pointless :P

		// The base class of lru_cache that defines the data members.
		template<typename key_t, typename value_t, const std::size_t max_size>
		class lru_cache_storage_base {
			#if LRU_CACHE_HASH_MAP_IMPLEMENTATION == STL_UNORDERED_MAP
				template<typename kt, typename vt>
				using map_t = std::unordered_map<kt, vt>;
			#elif LRU_CACHE_HASH_MAP_IMPLEMENTATION == ABSEIL_FLAT_HASH_MAP
				template<typename kt, typename vt>
				using map_t = absl::flat_hash_map<kt, vt>;
			#endif

		protected:
			using key_value_pair_t = std::pair<key_t, value_t>;
			using list_index_t = typename vector_list<key_value_pair_t, max_size>::index_t;
			using map_iterator_t = typename map_t<key_t, list_index_t>::iterator;

		public:
			using const_iterator = typename vector_list<key_value_pair_t, max_size>::const_iterator;
			using const_reverse_iterator = typename vector_list<key_value_pair_t, max_size>::const_reverse_iterator;

		protected:
			vector_list<key_value_pair_t, max_size> _cache_items_list;
			map_t<key_t, list_index_t> _cache_items_map;
		};

		// The base class of lru_cache that defines the default constructors, destructor and assignments.
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

		// The base class of lru_cache that defines the custom empty constructor and other default constructors, destructor and assignments.
		template<typename key_t, typename value_t, const std::size_t max_size>
		class lru_cache_base<key_t, value_t, max_size, true> : protected lru_cache_storage_base<key_t, value_t, max_size> {
		public:
			using key_value_pair_t = typename lru_cache_storage_base<key_t, value_t, max_size>::key_value_pair_t;

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

	// A fixed size (if preallocate is true) or bounded (if preallocate is false) container that
	// contains key-value pairs with unique keys. Search, insertion, and removal of elements have
	// average constant-time complexity. Two keys are considered equivalent if the KeyEqual<key_t>
	// predicate returns true when passed those keys. If two keys are equivalent, the Hash<key_t>
	// hash function must return the same value for both keys. lru_cache can not be created and
	// used in the evaluation of a constant expression (constexpr).
	// When filled, the container uses the Least Recently Used replacement policy to store subsequent elements.
	// Remarks:
	//   - The removed elements are not deleted immediately, instead they are replaced when new elements are put into the container.
	//   - When using std::unordered_map:
	//     - lru_cache is default constructible only if preallocate is false.
	//     - lru_cache is not trivially (default) constructible.
	//     - lru_cache is nothrow (default) constructible only if preallocate is false.
	//     - lru_cache is copy constructible.
	//     - lru_cache is not trivially copy constructible.
	//     - lru_cache is not nothrow copy constructible.
	//     - lru_cache is move constructible.
	//     - lru_cache is not trivially move constructible.
	//     - lru_cache is nothrow move constructible.
	//   - When using absl::flat_hash_map:
	//     - lru_cache is default constructible only if preallocate is false.
	//     - lru_cache is not trivially (default) constructible.
	//     - lru_cache is not nothrow (default) constructible.
	//     - lru_cache is copy constructible.
	//     - lru_cache is not trivially copy constructible.
	//     - lru_cache is not nothrow copy constructible.
	//     - lru_cache is move constructible.
	//     - lru_cache is not trivially move constructible.
	//     - lru_cache is nothrow move constructible.
	template<typename key_t, typename value_t, const std::size_t max_size, const bool preallocate = false>
	class lru_cache final : private detail::lru_cache_base<key_t, value_t, max_size, preallocate> {
		static_assert(max_size != 0u, "max_size can not be 0");

		using key_value_pair_t = typename detail::lru_cache_storage_base<key_t, value_t, max_size>::key_value_pair_t;
		using list_index_t = typename detail::lru_cache_storage_base<key_t, value_t, max_size>::list_index_t;
		using map_iterator_t = typename detail::lru_cache_storage_base<key_t, value_t, max_size>::map_iterator_t;

	public:
		using const_iterator = typename detail::lru_cache_storage_base<key_t, value_t, max_size>::const_iterator;
		using const_reverse_iterator = typename detail::lru_cache_storage_base<key_t, value_t, max_size>::const_reverse_iterator;

		// If the key already exists in the container, copies value to the mapped value inside the container.
		// If the key doesn't exist in the container, inserts a copy of value into the container.
		// If a reallocation takes place after the operation, or the container size is already at max_size,
		// all references and iterators are invalidated.
		void put(const key_t& key, const value_t& value) {
			map_iterator_t it = this->_cache_items_map.find(key);

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

		// If the key already exists in the container, moves value to the mapped value inside the container.
		// If the key doesn't exist in the container, inserts value into the container.
		// If a reallocation takes place after the operation, or the container size is already at max_size,
		// all references and iterators are invalidated.
		void put(const key_t& key, value_t&& value) {
			map_iterator_t it = this->_cache_items_map.find(key);

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

		// If the key already exists in the container, constructs a new element in-place with the given value_args into the mapped value inside the container.
		// If the key doesn't exist in the container, constructs a new element in-place with the given value_args into the container.
		// If a reallocation takes place after the operation, or the container size is already at max_size,
		// all references and iterators are invalidated.
		template<typename... ValueArgs>
		const value_t& emplace(const key_t& key, ValueArgs&&... value_args) {
			map_iterator_t it = this->_cache_items_map.find(key);

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

		// Returns a std::optional with the value that is mapped to the given key,
		// or an empty optional if such key does not already exist.
		const std::optional<value_t> get(const key_t& key) {
			map_iterator_t it = this->_cache_items_map.find(key);

			if (it == this->_cache_items_map.end()) {
				return std::nullopt;
			} else {
				return this->_cache_items_list._move_value_at_to_front(it->second).second;
			}
		}

		// Returns a std::optional with a reference to the value that is mapped to
		// the given key, or an empty optional if such key does not already exist.
		// Remarks:
		//   - No guarantees are given about the underlying object lifetime when
		//     modifying the cache (inserting/removing elements), so use with caution.
		const std::optional<std::reference_wrapper<const value_t>> get_ref(const key_t& key) {
			map_iterator_t it = this->_cache_items_map.find(key);

			if (it == this->_cache_items_map.end()) {
				return std::nullopt;
			} else {
				return std::make_optional(std::cref(this->_cache_items_list._move_value_at_to_front(it->second).second));
			}
		}

		// Returns true and copies the value that is mapped to the given key into
		// the given value_out reference, or false if such key does not already exist.
		bool try_get(const key_t& key, value_t& value_out) {
			map_iterator_t it = this->_cache_items_map.find(key);

			if (it == this->_cache_items_map.end()) {
				return false;
			} else {
				value_out = this->_cache_items_list._move_value_at_to_front(it->second).second;
				return true;
			}
		}

		// Returns true and assigns the address to the value that is mapped to the
		// given key into the given value_out pointer reference, or false if such
		// key does not already exist.
		// Remarks:
		//   - No guarantees are given about the underlying object lifetime when
		//     modifying the cache (inserting/removing elements), so use with caution.
		bool try_get_ref(const key_t& key, const value_t*& value_out) {
			map_iterator_t it = this->_cache_items_map.find(key);

			if (it == this->_cache_items_map.end()) {
				value_out = nullptr;
				return false;
			} else {
				value_out = &(this->_cache_items_list._move_value_at_to_front(it->second).second);
				return true;
			}
		}

		// If the key exists in the container, removes the value that is mapped to the
		// given key and returns a std::optional with the removed value,
		// otherwise, returns an empty optional.
		std::optional<value_t> remove(const key_t& key) {
			map_iterator_t it = this->_cache_items_map.find(key);

			if (it != this->_cache_items_map.end()) {
				value_t& value = this->_cache_items_list._erase_value_at(it->second).second;
				this->_cache_items_map.erase(it);
				return value;
			} else {
				return std::nullopt;
			}
		}

		// If the key exists in the container, removes the value that is mapped to the
		// given key and returns a std::optional with a reference to the removed value,
		// otherwise, returns an empty optional.
		// Remarks:
		//   - No guarantees are given about the underlying object lifetime when
		//     modifying the cache (inserting/removing elements), so use with caution.
		std::optional<std::reference_wrapper<value_t>> remove_ref(const key_t& key) {
			map_iterator_t it = this->_cache_items_map.find(key);

			if (it != this->_cache_items_map.end()) {
				value_t& value = this->_cache_items_list._erase_value_at(it->second).second;
				this->_cache_items_map.erase(it);
				return std::make_optional(std::ref(value));
			} else {
				return std::nullopt;
			}
		}

		// If the key exists in the container, removes the value that is mapped to the
		// given key, returns true and copies the value that is mapped to the given key
		// into the given value_out reference, or false if such key does not exist.
		bool try_remove(const key_t& key, value_t& value_out) {
			map_iterator_t it = this->_cache_items_map.find(key);

			if (it != this->_cache_items_map.end()) {
				value_out = this->_cache_items_list._erase_value_at(it->second).second;
				this->_cache_items_map.erase(it);
				return true;
			} else {
				return false;
			}
		}

		// If the key exists in the container, removes the value that is mapped to the
		// given key, returns true and assigns the address to the value that is mapped
		// to the given key into the given value_out pointer reference, or false if
		// such key does not exist.
		// Remarks:
		//   - No guarantees are given about the underlying object lifetime when
		//     modifying the cache (inserting/removing elements), so use with caution.
		bool try_remove_ref(const key_t& key, const value_t*& value_out) {
			map_iterator_t it = this->_cache_items_map.find(key);

			if (it != this->_cache_items_map.end()) {
				value_out = &(this->_cache_items_list._erase_value_at(it->second).second);
				this->_cache_items_map.erase(it);
				return true;
			} else {
				return false;
			}
		}

		// If the key exists in the container, removes the value that is mapped to the
		// given key, or false if such key does not exist.
		bool erase(const key_t& key) {
			map_iterator_t it = this->_cache_items_map.find(key);

			if (it != this->_cache_items_map.end()) {
				this->_cache_items_list._erase_value_at(it->second);
				this->_cache_items_map.erase(it);
				return true;
			} else {
				return false;
			}
		}

		// Checks if the container contains an element with the given key.
		bool exists(const key_t& key) const {
			return this->_cache_items_map.find(key) != this->_cache_items_map.end();
		}

		// Returns the number of elements in the container.
		std::size_t size() const noexcept {
			return this->_cache_items_map.size();
		}

		// Erases all elements from the container. After this call, size() returns zero.
		// References referring to contained elements are not invalidated, since the elements are deleted lazily.
		// Invalidates any iterators referring to contained elements.
		void clear() noexcept {
			this->_cache_items_map.clear();
			this->_cache_items_list.clear();
		}

		// Preallocates memory for max_size elements.
		// Remarks:
		//   - If std::unordered_map is used, only reserves capacity for the buckets. Element insertions will still cause some allocations.
		//   - If absl::flat_hash_map is used, this should prevent reallocations.
		void reserve() {
			this->_cache_items_map.reserve(max_size);
			this->_cache_items_list.reserve(max_size);
		}

		// Returns a const iterator to the first (most recently used) element of the container.
		// Remarks:
		//   - Accessing elements through iterators does not change their order of replacement.
		const_iterator cbegin() const noexcept {
			return this->_cache_items_list.cbegin();
		}

		// Returns a const iterator to the first (most recently used) element of the container.
		// Remarks:
		//   - Equivalent to cbegin().
		//   - Accessing elements through iterators does not change their order of replacement.
		const_iterator begin() const noexcept {
			return cbegin();
		}

		// Returns a const iterator to an invalid element of the container.
		// This returned iterator only acts as a sentinel. It is not guaranteed to be dereferenceable.
		// Remarks:
		//   - Accessing elements through iterators does not change their order of replacement.
		const_iterator cend() const noexcept {
			return this->_cache_items_list.cend();
		}

		// Returns a const iterator to an invalid element of the container.
		// This returned iterator only acts as a sentinel. It is not guaranteed to be dereferenceable.
		// Remarks:
		//   - Equivalent to cend().
		//   - Accessing elements through iterators does not change their order of replacement.
		const_iterator end() const noexcept {
			return cend();
		}

		// Returns a const reverse iterator to the first element of the reversed container.
		// It corresponds to the last (least recently used) element of the non-reversed container.
		// Remarks:
		//   - Accessing elements through iterators does not change their order of replacement.
		const_reverse_iterator crbegin() const noexcept {
			return this->_cache_items_list.crbegin();
		}

		// Returns a const reverse iterator to the first element of the reversed container.
		// It corresponds to the last (least recently used) element of the non-reversed container.
		// Remarks:
		//   - Equivalent to crbegin().
		//   - Accessing elements through iterators does not change their order of replacement.
		const_iterator rbegin() const noexcept {
			return crbegin();
		}

		// Returns a const reverse iterator to an invalid element of the container.
		// It corresponds to the element preceding the first element of the non-reversed container.
		// This returned iterator only acts as a sentinel. It is not guaranteed to be dereferenceable.
		// Remarks:
		//   - Accessing elements through iterators does not change their order of replacement.
		const_reverse_iterator crend() const noexcept {
			return this->_cache_items_list.crend();
		}

		// Returns a const reverse iterator to an invalid element of the container.
		// It corresponds to the element preceding the first element of the non-reversed container.
		// This returned iterator only acts as a sentinel. It is not guaranteed to be dereferenceable.
		// Remarks:
		//   - Equivalent to crbegin().
		//   - Accessing elements through iterators does not change their order of replacement.
		const_iterator rend() const noexcept {
			return crend();
		}
	};
}

// Restore old definitions if they were already defined
	// STL std::unordered_map
	#ifdef GUIORGY_LRU_CACHE_STL_UNORDERED_MAP_BEFORE
		#define STL_UNORDERED_MAP GUIORGY_LRU_CACHE_STL_UNORDERED_MAP_BEFORE
		#undef GUIORGY_LRU_CACHE_STL_UNORDERED_MAP_BEFORE
	#endif

	// STL std::unordered_map
	#ifdef GUIORGY_LRU_CACHE_ABSEIL_FLAT_HASH_MAP_BEFORE
		#define ABSEIL_FLAT_HASH_MAP GUIORGY_LRU_CACHE_ABSEIL_FLAT_HASH_MAP_BEFORE
		#undef GUIORGY_LRU_CACHE_ABSEIL_FLAT_HASH_MAP_BEFORE
	#endif
