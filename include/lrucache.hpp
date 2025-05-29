/*
** File: lrucache.hpp
**
** Author: Alexander Ponomarev
** Created on June 20, 2013, 5:09 PM
**
** Author: Guiorgy
** Forked on October 23, 2024
*/

#pragma once

#include <type_traits>
#include <functional>
#include <optional>
#include <cassert>
#include <cstdint>
#include <cstddef>
#include <utility>
#include <limits>
#include <vector>

// A helper macro to drop the explanation argument of the [[nodiscard]] attribute on C++ versions that don't support it.
#if __cplusplus < 202002L	// C++20
	#ifdef nodiscard
		#define GUIORGY_nodiscard_BEFORE
		#undef nodiscard
	#endif
	#define nodiscard(explanation) nodiscard
#endif

// Helper macros to remove the [[likely]] and [[unlikely]] attributes if C++ version < 20.
#ifdef LIKELY
	#define GUIORGY_LIKELY_BEFORE
	#undef LIKELY
#endif
#ifdef UNLIKELY
	#define GUIORGY_UNLIKELY_BEFORE
	#undef UNLIKELY
#endif
#if __cplusplus >= 202002L	// C++20
	#define LIKELY [[likely]]
	#define UNLIKELY [[unlikely]]
#else
	#define LIKELY
	#define UNLIKELY
#endif

/* Define possible implementations and store old definitions to restore later in case of a conflict */
// STL std::unordered_map
#ifdef STL_UNORDERED_MAP
	#define GUIORGY_LRU_CACHE_STL_UNORDERED_MAP_BEFORE STL_UNORDERED_MAP
	#undef STL_UNORDERED_MAP
#endif
#define STL_UNORDERED_MAP 10
// STL std::pmr::unordered_map with std::pmr::unsynchronized_pool_resource
#ifdef STL_PMR_UNORDERED_MAP
	#define GUIORGY_LRU_CACHE_STL_PMR_UNORDERED_MAP_BEFORE STL_PMR_UNORDERED_MAP
	#undef STL_PMR_UNORDERED_MAP
#endif
#define STL_PMR_UNORDERED_MAP 11

// Abseil absl::flat_hash_map
#ifdef ABSEIL_FLAT_HASH_MAP
	#define GUIORGY_LRU_CACHE_ABSEIL_FLAT_HASH_MAP_BEFORE ABSEIL_FLAT_HASH_MAP
	#undef ABSEIL_FLAT_HASH_MAP
#endif
#define ABSEIL_FLAT_HASH_MAP 20

// Tessil tsl::sparse_map
#ifdef TESSIL_SPARSE_MAP
	#define GUIORGY_LRU_CACHE_TESSIL_SPARSE_MAP_BEFORE TESSIL_SPARSE_MAP
	#undef TESSIL_SPARSE_MAP
#endif
#define TESSIL_SPARSE_MAP 30
// Tessil tsl::robin_map
#ifdef TESSIL_ROBIN_MAP
	#define GUIORGY_LRU_CACHE_TESSIL_ROBIN_MAP_BEFORE TESSIL_ROBIN_MAP
	#undef TESSIL_ROBIN_MAP
#endif
#define TESSIL_ROBIN_MAP 31
// Tessil tsl::hopscotch_map
#ifdef TESSIL_HOPSCOTCH_MAP
	#define GUIORGY_LRU_CACHE_TESSIL_HOPSCOTCH_MAP_BEFORE TESSIL_HOPSCOTCH_MAP
	#undef TESSIL_HOPSCOTCH_MAP
#endif
#define TESSIL_HOPSCOTCH_MAP 32

// Ankerl ankerl::unordered_dense::map
#ifdef ANKERL_UNORDERED_DENSE_MAP
	#define GUIORGY_LRU_CACHE_ANKERL_UNORDERED_DENSE_MAP_BEFORE ANKERL_UNORDERED_DENSE_MAP
	#undef ANKERL_UNORDERED_DENSE_MAP
#endif
#define ANKERL_UNORDERED_DENSE_MAP 40
// Ankerl ankerl::unordered_dense::segmented_map
#ifdef ANKERL_UNORDERED_DENSE_SEGMENTED_MAP
	#define GUIORGY_LRU_CACHE_ANKERL_UNORDERED_DENSE_SEGMENTED_MAP_BEFORE ANKERL_UNORDERED_DENSE_SEGMENTED_MAP
	#undef ANKERL_UNORDERED_DENSE_SEGMENTED_MAP
#endif
#define ANKERL_UNORDERED_DENSE_SEGMENTED_MAP 41

// Set the default implementation
#ifndef LRU_CACHE_HASH_MAP_IMPLEMENTATION
	#define LRU_CACHE_HASH_MAP_IMPLEMENTATION STL_UNORDERED_MAP
#endif

// Use the correct headers for the selected implementation
#if LRU_CACHE_HASH_MAP_IMPLEMENTATION == STL_UNORDERED_MAP
	// STL std::unordered_map
	#include <unordered_map>

	#ifdef LRU_CACHE_PRINT_HASH_MAP_IMPLEMENTATION
		#pragma message("Using std::unordered_map as the hashmap for the LRU cache")
	#endif
#elif LRU_CACHE_HASH_MAP_IMPLEMENTATION == STL_PMR_UNORDERED_MAP
	// STL std::pmr::unordered_map with std::pmr::unsynchronized_pool_resource
	#include <memory_resource>
	#include <unordered_map>

	#ifdef LRU_CACHE_PRINT_HASH_MAP_IMPLEMENTATION
		#pragma message("Using std::pmr::unordered_map as the hashmap for the LRU cache")
	#endif
#elif LRU_CACHE_HASH_MAP_IMPLEMENTATION == ABSEIL_FLAT_HASH_MAP
	// Abseil absl::flat_hash_map
	#include "absl/container/flat_hash_map.h"

	#ifdef LRU_CACHE_PRINT_HASH_MAP_IMPLEMENTATION
		#pragma message("Using absl::flat_hash_map as the hashmap for the LRU cache")
	#endif
#elif LRU_CACHE_HASH_MAP_IMPLEMENTATION == TESSIL_SPARSE_MAP
	// Tessil tsl::sparse_map
	#include "tsl/sparse_map.h"

	#ifdef LRU_CACHE_PRINT_HASH_MAP_IMPLEMENTATION
		#pragma message("Using tsl::sparse_map as the hashmap for the LRU cache")
	#endif
#elif LRU_CACHE_HASH_MAP_IMPLEMENTATION == TESSIL_ROBIN_MAP
	// Tessil tsl::robin_map
	#include "tsl/robin_map.h"

	#ifdef LRU_CACHE_PRINT_HASH_MAP_IMPLEMENTATION
		#pragma message("Using tsl::robin_map as the hashmap for the LRU cache")
	#endif
#elif LRU_CACHE_HASH_MAP_IMPLEMENTATION == TESSIL_HOPSCOTCH_MAP
	// Tessil tsl::hopscotch_map
	#include "tsl/hopscotch_map.h"

	#ifdef LRU_CACHE_PRINT_HASH_MAP_IMPLEMENTATION
		#pragma message("Using tsl::hopscotch_map as the hashmap for the LRU cache")
	#endif
#elif LRU_CACHE_HASH_MAP_IMPLEMENTATION == ANKERL_UNORDERED_DENSE_MAP
	// Ankerl ankerl::unordered_dense::map
	#include "ankerl/unordered_dense.h"

	#ifdef LRU_CACHE_PRINT_HASH_MAP_IMPLEMENTATION
		#pragma message("Using ankerl::unordered_dense::map as the hashmap for the LRU cache")
	#endif
#elif LRU_CACHE_HASH_MAP_IMPLEMENTATION == ANKERL_UNORDERED_DENSE_SEGMENTED_MAP
	// Ankerl ankerl::unordered_dense::segmented_map
	#include "ankerl/unordered_dense.h"

	#ifdef LRU_CACHE_PRINT_HASH_MAP_IMPLEMENTATION
		#pragma message("Using ankerl::unordered_dense::segmented_map as the hashmap for the LRU cache")
	#endif
#else
	#ifdef VALUE_TO_STRING
		#undef VALUE_TO_STRING
	#endif
	#ifdef VALUE
		#undef VALUE
	#endif
	#define VALUE_TO_STRING(x) #x
	#define VALUE(x) VALUE_TO_STRING(x)

	#pragma message("LRU_CACHE_HASH_MAP_IMPLEMENTATION is set to " VALUE(LRU_CACHE_HASH_MAP_IMPLEMENTATION))
	#pragma message("Possible valiues are STL_UNORDERED_MAP(std::unordered_map), STL_PMR_UNORDERED_MAP(std::pmr::unordered_map), ABSEIL_FLAT_HASH_MAP(absl::flat_hash_map), TESSIL_SPARSE_MAP(tsl::sparse_map), TESSIL_ROBIN_MAP(tsl::robin_map), TESSIL_HOPSCOTCH_MAP(tsl::hopscotch_map), ANKERL_UNORDERED_DENSE_MAP(ankerl::unordered_dense::map), ANKERL_UNORDERED_DENSE_SEGMENTED_MAP(ankerl::unordered_dense::segmented_map)")
	#error "Unexpected value of LRU_CACHE_HASH_MAP_IMPLEMENTATION"
#endif

// Type trait utils.
namespace guiorgy::detail {
	// Determines the smallest unsigned integer type that can fit the specified value
	// if fast is set to false, otherwise, the fastest unsigned integer type that can
	// fit specified value.
	template <const std::size_t max_value, const bool fast = false>
	struct uint_fit final {
		static_assert(max_value >= 0u, "std::size_t is less than 0?!");
		static_assert(max_value <= std::numeric_limits<std::uint64_t>::max(), "uint_fit only supports up to 64 bit numbers");

		using type = std::conditional_t<
			max_value <= std::numeric_limits<std::uint8_t>::max(),
			std::conditional_t<fast, std::uint_fast8_t, std::uint8_t>,
			std::conditional_t<
				max_value <= std::numeric_limits<std::uint16_t>::max(),
				std::conditional_t<fast, std::uint_fast16_t, std::uint16_t>,
				std::conditional_t<
					max_value <= std::numeric_limits<std::uint32_t>::max(),
					std::conditional_t<fast, std::uint_fast32_t, std::uint32_t>,
					std::conditional_t<fast, std::uint_fast64_t, std::uint64_t>
				>
			>
		>;
	};

	// Helper for uint_fit.
	template<const std::size_t max_value, const bool fast = false>
	using uint_fit_t = typename uint_fit<max_value, fast>::type;

	// See uint_fit for details.
	template<const std::size_t max_value>
	using uint_fit_fast = uint_fit<max_value, true>;
	template<const std::size_t max_value>
	using uint_fit_fast_t = typename uint_fit_fast<max_value>::type;

	// SFINAE to check if the specified type is a std::pair.
	// The template returned when matching fails.
	template<typename, typename = void>
	struct is_pair : std::false_type {};
	// The template returned when matching succeeds.
	template<typename T>
	struct is_pair<
		T,
		std::void_t<
			typename T::first_type,
			typename T::second_type
		>
	> final : std::is_same<T, std::pair<typename T::first_type, typename T::second_type>> {};
	// Helper for is_pair.
	template<typename T>
	inline constexpr bool is_pair_v = is_pair<T>::value;

	// SFINAE to check if the specified type has an emplace_hint member function similar to std::unordered_map.
	// The template returned when matching fails.
	template<typename, typename = void>
	struct has_emplace_hint final : std::false_type {};
	// The template returned when matching succeeds.
	template<typename T>
	struct has_emplace_hint<
		T,
		std::void_t<
			decltype(
				std::declval<T&>().emplace_hint(
					std::declval<typename T::const_iterator>(),
					std::declval<typename T::key_type&&>(),
					std::declval<typename T::mapped_type&&>()
				)
			)
		>
	> final : std::true_type {};
	// Helper for has_emplace_hint.
	template<typename T>
	inline constexpr bool has_emplace_hint_v = has_emplace_hint<T>::value;

	// SFINAE to check if the specified type has an insert member function that takes an iterator hint similar to std::unordered_map.
	// The template returned when matching fails.
	template<typename, typename = void>
	struct has_insert_with_hint final : std::false_type {};
	// The template returned when matching succeeds.
	template<typename T>
	struct has_insert_with_hint<
		T,
		std::void_t<
			typename std::enable_if_t<is_pair_v<typename T::value_type>>,
			decltype(
				std::declval<T&>().insert(
					std::declval<typename T::iterator>(),
					std::declval<typename T::value_type&&>()
				)
			)
		>
	> final : std::true_type {};
	// Helper for has_insert_with_hint.
	template<typename T>
	inline constexpr bool has_insert_with_hint_v = has_insert_with_hint<T>::value;
} // guiorgy::detail

// Utils.
namespace guiorgy::detail {
	// Emplaces a new object in place of the old.
	// If replace is set to false, assumes that the destination is uninitialized.
	// Remarks:
	//   - Calling emplace with an initialized object as the destination and
	//     replace explicitly set to false results in undefined behaviour.
	template<typename T, const bool replace = !std::is_trivially_destructible_v<T>, typename... Args>
	inline T& emplace(T* destination, Args&&... args)
		noexcept(
			(!replace || std::is_nothrow_destructible_v<T>)
			&& std::is_nothrow_constructible_v<T, Args...>
		) {
		assert(destination != nullptr);

		if constexpr (replace) {
			static_assert(std::is_destructible_v<T>, "T needs to be destructible to allow emplacement");

			(*destination).~T();
		}

		::new (destination) T(std::forward<Args>(args)...);
		return *destination;
	}

	// Emplaces a new object in place.
	// Remarks:
	//   - This assumes the destination is uninitialized. Calling emplace_new with
	//     an initialized object as the destination results in undefined behaviour.
	template<typename T, typename... Args>
	T& emplace_new = emplace<T, false, Args...>;

	// Decrements an integer by one, but clamps it to a minimum value if it would underflow it.
	template<typename int_t>
	[[nodiscard]] inline constexpr int_t clamped_decrement(int_t x, const int_t minimum) noexcept {
		static_assert(std::is_integral_v<int_t>, "clamped_decrement only accepts integrals");

		if (x > minimum) --x;
		return x;
	}

	// Decrements an integer by one, but only if this doesn't cause an underflow.
	template<typename int_t>
	[[nodiscard]] inline constexpr int_t safe_decrement(int_t x) noexcept {
		return clamped_decrement(x, std::numeric_limits<int_t>::lowest());
	}

	// Increments an integer by one, but clamps it to a maximum value if it would overflow it.
	template<typename int_t>
	[[nodiscard]] inline constexpr int_t clamped_increment(int_t x, const int_t maximum) noexcept {
		static_assert(std::is_integral_v<int_t>, "clamped_increment only accepts integrals");

		if (x < maximum) ++x;
		return x;
	}

	// Increments an integer by one, but only if this doesn't cause an overflow.
	template<typename int_t>
	[[nodiscard]] inline constexpr int_t safe_increment(const int_t x) noexcept {
		return clamped_increment(x, std::numeric_limits<int_t>::max());
	}
}

// Forward declarations.
namespace guiorgy {
	// Forward declaration of lru_cache.
	template<typename key_t, typename value_t, const std::size_t max_size, const bool preallocate, typename hash_function, typename key_equality_predicate>
	class lru_cache;

	namespace detail {
		// Forward declaration of lru_cache_storage_base.
		template<typename key_t, typename value_t, const std::size_t max_size, typename hash_function, typename key_equality_predicate>
		class lru_cache_storage_base;

		// Forward declaration of vector_list.
		template<typename T, const std::size_t max_size>
		class vector_list;
	} // detail
} // guiorgy

// Containers.
namespace guiorgy::detail {
	// A container where you can store elements and then take them out in an unspecified order.
	// Remarks:
	//   - The size of the container must not exceed the maximum value representable by index_t plus one.
	//   - The size limitation is not enforced within the container, the user must ensure that this condition is not violated.
	//   - The removed elements are not deleted immediately, instead they are replaced when new elements are put into the container.
	template<typename T, typename index_t = std::size_t>
	class vector_set final {
		std::vector<T> set{};
		index_t head = 0u;
		index_t tail = 0u;
		bool _empty = true;

		// Returns the index of the next element.
		template<const bool forward = true>
		[[nodiscard]] index_t next_index(const index_t index) const noexcept {
			assert(set.size() <= static_cast<std::size_t>(std::numeric_limits<index_t>::max()) + 1u);
			assert(set.size() != 0u);
			assert(static_cast<std::size_t>(index) < set.size());

			if constexpr (forward) {
				return index != static_cast<index_t>(safe_decrement(set.size())) ? index + 1u : 0u;
			} else {
				return index != 0u ? static_cast<index_t>(index - 1u) : static_cast<index_t>(safe_decrement(set.size()));
			}
		}

	public:
		// Increases the capacity of the set (the total number of elements that the set can hold without requiring a reallocation) to a value that's greater or equal to capacity.
		void reserve(const std::size_t capacity) {
			assert(capacity == 0u || capacity - 1u <= std::numeric_limits<index_t>::max());

			set.reserve(capacity);
		}

		// Checks whether the set is empty.
		[[nodiscard]] bool empty() const noexcept {
			return _empty;
		}

		// Returns the number of elements in the set.
		[[nodiscard]] std::size_t size() const noexcept {
			if (_empty) LIKELY {
				return 0u;
			} else {
				std::size_t _head = static_cast<std::size_t>(head);
				std::size_t _tail = static_cast<std::size_t>(tail);

				return _tail <= _head ? _head - tail + 1u : (_head + 1u) + (set.size() - tail);
			}
		}

		// Returns the number of elements that the set has currently allocated space for.
		[[nodiscard]] std::size_t capacity() const noexcept {
			return set.capacity();
		}

		// Puts the given element into the set.
		void put(const T& value) {
			if (_empty) LIKELY {
				assert(head == tail);

				_empty = false;

				if (set.empty()) UNLIKELY {
					assert(head == 0u);

					set.push_back(value);
				} else {
					assert(head < set.size());

					set[head] = value;
				}
			} else {
				index_t next_head = next_index(head);

				if (next_head == tail) UNLIKELY {
					assert(size() == set.size());

					set.push_back(value);

					if (tail == 0u) LIKELY {
						++head;
					}
				} else {
					head = next_head;
					set[head] = value;
				}
			}

			assert(set.size() == 0u || set.size() - 1u <= std::numeric_limits<index_t>::max());
		}

		// Puts the given element into the set.
		void put(T&& value) {
			if (_empty) LIKELY {
				assert(head == tail);

				_empty = false;

				if (set.empty()) UNLIKELY {
					assert(head == 0u);

					set.push_back(std::move(value));
				} else {
					assert(head < set.size());

					set[head] = std::move(value);
				}
			} else {
				index_t next_head = next_index(head);

				if (next_head == tail) UNLIKELY {
					assert(set.size() == set.size());

					set.push_back(std::move(value));

					if (tail == 0u) LIKELY {
						++head;
					}
				} else {
					head = next_head;
					set[head] = std::move(value);
				}
			}

			assert(set.size() == 0u || set.size() - 1u <= std::numeric_limits<index_t>::max());
		}

		// Emplaces the given element into the set.
		template<typename... ValueArgs>
		void emplace(ValueArgs&&... value_args) {
			if (_empty) LIKELY {
				assert(head == tail);

				_empty = false;

				if (set.empty()) UNLIKELY {
					assert(head == 0u);

					set.emplace_back(std::forward<ValueArgs>(value_args)...);
				} else {
					assert(head < set.size());

					emplace(&set[head], std::forward<ValueArgs>(value_args)...);
				}
			} else {
				index_t next_head = next_index(head);

				if (next_head == tail) UNLIKELY {
					assert(set.size() == set.size());

					set.emplace_back(std::forward<ValueArgs>(value_args)...);

					if (tail == 0u) LIKELY {
						++head;
					}
				} else {
					head = next_head;
					emplace(&set[head], std::forward<ValueArgs>(value_args)...);
				}
			}

			assert(set.size() == 0u || set.size() - 1u <= std::numeric_limits<index_t>::max());
		}

		// Returns a reference to the next element in the set.
		template<const bool from_head = true>
		[[nodiscard]] T& peek() const {
			assert(!_empty);

			if constexpr (from_head) {
				return set[head];
			} else {
				return set[tail];
			}
		}

		// Removes the next element in the set and returns a reference to it.
		// If from_head is always true, vector_set effectively acts like a vector backed stack.
		// References to the removed element are not invalidated, since the element is deleted lazily.
		template<const bool from_head = true>
		[[nodiscard]] T& take_ref() {
			assert(!_empty);

			if constexpr (from_head) {
				T& _head = set[head];
				if (head == tail) LIKELY _empty = true;
				else head = next_index<false>(head);
				return _head;
			} else {
				T& _tail = set[tail];
				if (tail == head) LIKELY _empty = true;
				else tail = next_index<true>(tail);
				return _tail;
			}
		}

		// Removes the next element in the set and returns it.
		// If from_head is always true, vector_set effectively acts like a vector backed stack.
		template<const bool from_head = true>
		[[nodiscard]] T take() {
			return take_ref<from_head>();
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
			static_assert(std::is_integral_v<T> && std::is_unsigned_v<T>, "This is only meant to be used by vector_list");

			if (count == 0u) UNLIKELY {
				clear();
				return;
			}

			assert(count - 1u <= std::numeric_limits<T>::max());

			const std::size_t _size = set.size();
			if (count <= _size) UNLIKELY {
				for (std::size_t i = 0u; i < count; ++i) {
					set[i] = static_cast<T>(i);
				}
			} else {
				reserve(count);

				for (std::size_t i = 0u; i < _size; ++i) {
					set[i] = static_cast<T>(i);
				}
				for (std::size_t i = _size; i < count; ++i) {
					set.push_back(static_cast<T>(i));
				}
			}

			tail = 0u;
			head = static_cast<T>(count - 1u);
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

			list_node(const T& _value, const index_t _prior, const index_t _next) :
				value(_value),
				prior(_prior),
				next(_next)
#ifndef NDEBUG
				, removed(false)
#endif
				{}

			list_node(T&& _value, const index_t _prior, const index_t _next) :
				value(std::move(_value)),
				prior(_prior),
				next(_next)
#ifndef NDEBUG
				, removed(false)
#endif
				{}

			template<typename... ValueArgs>
			list_node(const index_t _prior, const index_t _next, ValueArgs&&... value_args) :
				value(std::forward<ValueArgs>(value_args)...),
				prior(_prior),
				next(_next)
#ifndef NDEBUG
				, removed(false)
#endif
				{}

			template<typename... ValueArgs>
			T& emplace_value(ValueArgs&&... value_args) {
				return emplace(&value, std::forward<ValueArgs>(value_args)...);
			}

			list_node() = delete;
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
		std::vector<list_node> list{};
		index_t head = null_index;
		index_t tail = null_index;
		vector_set<index_t, index_t> free_indices{};

		// Returns a reference to the node at the specified location.
		[[nodiscard]] list_node& get_node(const index_t at) {
			assert(at < list.size());
#ifndef NDEBUG
			assert(!list[at].removed);
#endif

			return list[at];
		}

		// Returns a const reference to the node at the specified location.
		[[nodiscard]] const list_node& get_node(const index_t at) const {
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
		template<const bool mark_removed = true>
		[[nodiscard("Especially if mark_removed is set to false. Use erase_node(const index_t at) instead")]]
		list_node& remove_node(const index_t at) {
			assert(at < list.size());
#ifndef NDEBUG
			assert(!list[at].removed);
#endif

			list_node& _at = list[at];

			if (at != head && at != tail) LIKELY {
				assert(_at.prior != null_index && _at.next != null_index);

				list[_at.prior].next = _at.next;
				list[_at.next].prior = _at.prior;
			} else if (at != tail) LIKELY {
				assert(_at.prior != null_index && _at.next == null_index);

				head = _at.prior;
				list[head].next = null_index;
			} else if (at != head) UNLIKELY {
				assert(_at.prior == null_index && _at.next != null_index);

				tail = _at.next;
				list[tail].prior = null_index;
			} else {
				assert(_at.prior == null_index && _at.next == null_index);

				head = null_index;
				tail = null_index;
			}

			if constexpr (mark_removed) {
				free_indices.put(at);
#ifndef NDEBUG
				_at.removed = true;
#endif
			}

			return _at;
		}

		// Removes the node at the specified location.
		// References to the removed node are not invalidated, since the node is just marked as removed and added to the free_indices set.
		// Iterators to the removed node are invalidated. Other iterators are not affected.
		void erase_node(const index_t at) {
			[[maybe_unused]] list_node& discard = remove_node<true>(at);
		}

		// Moves the node at the specified location to before/after another node and returns a reference to the moved node.
		// References to the moved/destination node are not invalidated, since only the prior and next members of the node are updated.
		// Iterators to the moved node are invalidated. Other iterators, including the iterators to the node at the movement destination, are not affected.
		// If before is true, then the node is moved before the node at the destination, otherwise, the node is moved after the destination.
		template<const bool before = true>
		list_node& move_node(const index_t from, const index_t to) {
			assert(head != null_index);
			assert(from < list.size());
			assert(to < list.size());
#ifndef NDEBUG
			assert(!list[from].removed);
			assert(!list[to].removed);
#endif

			if (from == to) UNLIKELY return list[from];

			list_node& _from = remove_node<false>(from);
			list_node& _to = list[to];

			if constexpr (before) {
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

			if (from == head) UNLIKELY return list[head];

			list_node& _from = remove_node<false>(from);

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

			if (from == tail) UNLIKELY return list[tail];

			list_node& _from = remove_node<false>(from);

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
		[[nodiscard]] bool empty() const noexcept {
			return list.size() == free_indices.size();
		}

		// Returns the number of elements in the list.
		[[nodiscard]] std::size_t size() const noexcept {
			return list.size() - free_indices.size();
		}

		// Returns the number of elements that the list has currently allocated space for.
		[[nodiscard]] std::size_t capacity() const noexcept {
			return list.capacity();
		}

		// Prepends a copy of value to the beginning of the list.
		// If after the operation the new size() is greater than old capacity() a reallocation takes place, in which case all references are invalidated.
		// No iterators are invalidated.
		void push_front(const T& value) {
			if (!free_indices.empty()) UNLIKELY {
				index_t index = free_indices.take();
				list_node& node = list[index];

				node.value = value;
				node.prior = head;
				node.next = null_index;
#ifndef NDEBUG
				assert(node.removed);
				node.removed = false;
#endif

				if (head != null_index) LIKELY list[head].next = index;
				head = index;
				if (tail == null_index) UNLIKELY tail = head;
			} else {
				assert(list.size() <= std::numeric_limits<index_t>::max());
				index_t list_size = static_cast<index_t>(list.size());

				if (head != null_index) LIKELY list[head].next = list_size;
				index_t prior = head;
				head = list_size;
				if (tail == null_index) UNLIKELY tail = head;

				list.emplace_back(value, prior, null_index);
			}
		}

		// Prepends value to the beginning of the list.
		// If after the operation the new size() is greater than old capacity() a reallocation takes place, in which case all references are invalidated.
		// No iterators are invalidated.
		void push_front(T&& value) {
			if (!free_indices.empty()) UNLIKELY {
				index_t index = free_indices.take();
				list_node& node = list[index];

				node.value = std::move(value);
				node.prior = head;
				node.next = null_index;
#ifndef NDEBUG
				assert(node.removed);
				node.removed = false;
#endif

				if (head != null_index) LIKELY list[head].next = index;
				head = index;
				if (tail == null_index) UNLIKELY tail = head;
			} else {
				assert(list.size() <= std::numeric_limits<index_t>::max());
				index_t list_size = static_cast<index_t>(list.size());

				if (head != null_index) LIKELY list[head].next = list_size;
				index_t prior = head;
				head = list_size;
				if (tail == null_index) UNLIKELY tail = head;

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
			if (!free_indices.empty()) UNLIKELY {
				index_t index = free_indices.take();
				list_node& node = list[index];

				T& value = node.emplace_value(std::forward<ValueArgs>(value_args)...);
				node.prior = head;
				node.next = null_index;
#ifndef NDEBUG
				assert(node.removed);
				node.removed = false;
#endif

				if (head != null_index) LIKELY list[head].next = index;
				head = index;
				if (tail == null_index) UNLIKELY tail = head;

				return value;
			} else {
				assert(list.size() <= std::numeric_limits<index_t>::max());
				index_t list_size = static_cast<index_t>(list.size());

				if (head != null_index) LIKELY list[head].next = list_size;
				index_t prior = head;
				head = list_size;
				if (tail == null_index) UNLIKELY tail = head;

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

				if (tail != null_index) LIKELY list[tail].prior = index;
				tail = index;
				if (head == null_index) UNLIKELY head = tail;
			} else {
				assert(list.size() <= std::numeric_limits<index_t>::max());
				index_t list_size = static_cast<index_t>(list.size());

				if (tail != null_index) LIKELY list[tail].prior = list_size;
				index_t next = tail;
				tail = list_size;
				if (head == null_index) UNLIKELY head = tail;

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

				if (tail != null_index) LIKELY list[tail].prior = index;
				tail = index;
				if (head == null_index) UNLIKELY head = tail;
			} else {
				assert(list.size() <= std::numeric_limits<index_t>::max());
				index_t list_size = static_cast<index_t>(list.size());

				if (tail != null_index) LIKELY list[tail].prior = list_size;
				index_t next = tail;
				tail = list_size;
				if (head == null_index) UNLIKELY head = tail;

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

				if (tail != null_index) LIKELY list[tail].prior = index;
				tail = index;
				if (head == null_index) UNLIKELY head = tail;

				return value;
			} else {
				assert(list.size() <= std::numeric_limits<index_t>::max());
				index_t list_size = static_cast<index_t>(list.size());

				if (tail != null_index) LIKELY list[tail].prior = list_size;
				index_t next = tail;
				tail = list_size;
				if (head == null_index) UNLIKELY head = tail;

				return list.emplace_back(null_index, next, std::forward<ValueArgs>(value_args)...).value;
			}
		}

		// Returns a reference to the first element in the list.
		[[nodiscard]] T& front() {
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
			if (head == null_index) UNLIKELY tail = null_index;
			else list[head].next = null_index;
			return _front;
		}

		// Removes the first element of the list and returns a copy of ithe removed element.
		// References to the removed element are not invalidated, since the element is deleted lazily.
		// Iterators to the removed element are invalidated. Other iterators are not affected.
		[[nodiscard("Use pop_front_ref() instead to avoid a needless copy of T")]]
		T pop_front() {
			return pop_front_ref();
		}

		// Returns a reference to the last element in the list.
		[[nodiscard]] T& back() {
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
			if (tail == null_index) UNLIKELY head = null_index;
			else list[tail].next = null_index;
			return _back;
		}

		// Removes the last element of the list and returns a copy of ithe removed element.
		// References to the removed element are not invalidated, since the element is deleted lazily.
		// Iterators to the removed element are invalidated. Other iterators are not affected.
		[[nodiscard("Use pop_back_ref() instead to avoid a needless copy of T")]]
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
			return remove_node<true>(it.forward_index()).value;
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
		[[nodiscard]] iterator begin() noexcept {
			return iterator(*this, head);
		}

		// Returns an iterator to an invalid element of the list.
		// This returned iterator only acts as a sentinel. It is not guaranteed to be dereferenceable.
		[[nodiscard]] iterator end() noexcept {
			return iterator(*this, null_index);
		}

		// Returns a const iterator to the front element of the list.
		[[nodiscard]] const_iterator begin() const noexcept {
			return const_iterator(*this, head);
		}

		// Returns a const iterator to an invalid element of the list.
		// This returned iterator only acts as a sentinel. It is not guaranteed to be dereferenceable.
		[[nodiscard]] const_iterator end() const noexcept {
			return const_iterator(*this, null_index);
		}

		// Returns a const iterator to the front element of the list.
		[[nodiscard]] const_iterator cbegin() const noexcept {
			return const_iterator(*this, head);
		}

		// Returns a const iterator to an invalid element of the list.
		// This returned iterator only acts as a sentinel. It is not guaranteed to be dereferenceable.
		[[nodiscard]] const_iterator cend() const noexcept {
			return const_iterator(*this, null_index);
		}

		// Returns a reverse iterator to the front element of the reversed list.
		// It corresponds to the back element of the non-reversed list.
		[[nodiscard]] reverse_iterator rbegin() noexcept {
			return reverse_iterator(*this, null_index);
		}

		// Returns a reverse iterator to an invalid element of the list.
		// It corresponds to the element preceding the first element of the non-reversed list.
		// This returned iterator only acts as a sentinel. It is not guaranteed to be dereferenceable.
		[[nodiscard]] reverse_iterator rend() noexcept {
			return reverse_iterator(*this, head);
		}

		// Returns a const reverse iterator to the front element of the reversed list.
		// It corresponds to the back element of the non-reversed list.
		[[nodiscard]] const_reverse_iterator rbegin() const noexcept {
			return const_reverse_iterator(*this, null_index);
		}

		// Returns a const reverse iterator to an invalid element of the list.
		// It corresponds to the element preceding the first element of the non-reversed list.
		// This returned iterator only acts as a sentinel. It is not guaranteed to be dereferenceable.
		[[nodiscard]] const_reverse_iterator rend() const noexcept {
			return const_reverse_iterator(*this, head);
		}

		// Returns a const reverse iterator to the front element of the reversed list.
		// It corresponds to the back element of the non-reversed list.
		[[nodiscard]] const_reverse_iterator crbegin() const noexcept {
			return const_reverse_iterator(*this, null_index);
		}

		// Returns a const reverse iterator to an invalid element of the list.
		// It corresponds to the element preceding the first element of the non-reversed list.
		// This returned iterator only acts as a sentinel. It is not guaranteed to be dereferenceable.
		[[nodiscard]] const_reverse_iterator crend() const noexcept {
			return const_reverse_iterator(*this, head);
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
			using list_reference_t = std::conditional_t<constant, const vector_list<T, max_size>&, vector_list<T, max_size>&>;
			using list_pointer_t = std::conditional_t<constant, const vector_list<T, max_size>*, vector_list<T, max_size>*>;

			list_pointer_t list;
			index_t current_index;

			_iterator(list_reference_t _list, const index_t _index) : list(&_list), current_index(_index) {}

			template<const bool from_reverse, const bool to_reverse>
			static index_t reverse_index(list_pointer_t _list, const index_t index) {
				static_assert(from_reverse != to_reverse);

				if constexpr (!from_reverse) {
					return index != null_index ? (*_list)[index].prior : null_index;
				} else {
					return index == null_index ? _list->tail : (*_list)[index].next;
				}
			}

		public:
			// Copy constructor from (reverse_)iterator to const_(reverse_)iterator.
			template<const bool other_constant>
			_iterator(const _iterator<other_constant, reverse>& it) : list(it.list), current_index(it.current_index) {}

			// Copy constructor from (const_)iterator to (const_)reverse_iterator and from (const_)reverse_iterator to (const_)iterator.
			template<const bool other_reverse>
			_iterator(const _iterator<constant, other_reverse>& it) : list(it.list), current_index(reverse_index<other_reverse, reverse>(it.list, it.current_index)) {}

			_iterator() = delete;
			~_iterator() = default;
			_iterator(const _iterator&) = default;
			_iterator(_iterator&&) = default;
			_iterator& operator=(_iterator const&) = default;
			_iterator& operator=(_iterator &&) = default;

		private:
			// Normalizes the index to the one used in forward iteration.
			// In other words, if the current iterator is a (const_)reverse_iterator this returns the index to the list node this iterator actually refers to.
			[[nodiscard]] index_t forward_index() const noexcept {
				if constexpr (!reverse) {
					return current_index;
				} else {
					return current_index == null_index ? list->tail : (*list)[current_index].next;
				}
			}

		public:
			reference operator*() const {
				return (*list)[forward_index()].value;
			}

			pointer operator->() const {
				return &((*list)[forward_index()].value);
			}

			_iterator& operator++() {
				if constexpr (!reverse) {
					if (current_index != null_index) current_index = (*list)[current_index].prior;
				} else {
					if (current_index != list->head) current_index = current_index == null_index ? list->tail : (*list)[current_index].next;
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
					if (current_index != list->head) current_index = current_index == null_index ? list->tail : (*list)[current_index].next;
				} else {
					if (current_index != null_index) current_index = (*list)[current_index].prior;
				}

				return *this;
			}

			_iterator operator--(int) {
				_iterator temp = *this;
				--(*this);
				return temp;
			}

			[[nodiscard]] bool operator==(const _iterator& other) const noexcept {
				return current_index == other.current_index;
			}

			[[nodiscard]] bool operator!=(const _iterator& other) const noexcept {
				return !(*this == other);
			}

		private:
			friend class vector_list<T, max_size>;
		};

	private:
		// See get_node for details.
		// This should only be used by _iterator;
		[[nodiscard]] list_node& operator[](const index_t position) {
			return get_node(position);
		}

		// See get_node for details.
		// This should only be used by _iterator;
		[[nodiscard]] const list_node& operator[](const index_t position) const {
			return get_node(position);
		}

		// Declare lru_cache_storage_base as a friend to allow access to index_t.
		template<typename kt, typename vt, const std::size_t ms, typename hf, typename kep>
		friend class lru_cache_storage_base;

		// Declare lru_cache as a friend to give access to the dangerous member functions below.
		template<typename kt, typename vt, const std::size_t ms, const bool p, typename hf, typename kep>
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
		[[nodiscard]] T& _get_value_at(const index_t position) {
			return get_node(position).value;
		}

		// See remove_node for details.
		T& _erase_value_at(const index_t position) {
			return remove_node<true>(position).value;
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
		[[nodiscard]] index_t _first_value_index() const noexcept {
			assert(head != null_index);

			return head;
		}

		// Returns the index to the last element in the list, or null_index if the list is empty.
		[[nodiscard]] index_t _last_value_index() const noexcept {
			assert(tail != null_index);

			return tail;
		}
	};
} // guiorgy::detail

// lru_cache details.
namespace guiorgy::detail {
	// Placeholders to indicate that the underlying hashmap should use its default hash function and key equality predicate.
	template<typename key_t>
	class DefaultHashFunction final {};
	template<typename key_t>
	class DefaultKeyEqualityPredicate final {};

	// lru_cache_storage_base and lru_cache_base(<key_t, value_t, max_size>) are used
	// to allow the conditional declaration of a default or custom constructor in C++ 17.
	// For more details see: https://devblogs.microsoft.com/cppblog/conditionally-trivial-special-member-functions/
	// Although, turns out std::vector is not trivially (default) constructible anyway, so this is actually pointless :P

	// The base class of lru_cache that defines the data members.
	template<typename key_t, typename value_t, const std::size_t max_size, typename hash_function, typename key_equality_predicate>
	class lru_cache_storage_base {
		static_assert(std::is_same_v<key_equality_predicate, DefaultKeyEqualityPredicate<key_t>> || !std::is_same_v<hash_function, DefaultHashFunction<key_t>>, "hash_function can't be default if key_equality_predicate is not default");

		#ifdef STL_UNORDERED_MAP_LIKE_QUALIFIED_TYPE
			#define STL_UNORDERED_MAP_LIKE_QUALIFIED_TYPE_BEFORE STL_UNORDERED_MAP_LIKE_QUALIFIED_TYPE
			#undef STL_UNORDERED_MAP_LIKE_QUALIFIED_TYPE
		#endif
		#if LRU_CACHE_HASH_MAP_IMPLEMENTATION == STL_UNORDERED_MAP
			#define STL_UNORDERED_MAP_LIKE_QUALIFIED_TYPE std::unordered_map
		#elif LRU_CACHE_HASH_MAP_IMPLEMENTATION == STL_PMR_UNORDERED_MAP
			#define STL_UNORDERED_MAP_LIKE_QUALIFIED_TYPE std::pmr::unordered_map
		#elif LRU_CACHE_HASH_MAP_IMPLEMENTATION == ABSEIL_FLAT_HASH_MAP
			#define STL_UNORDERED_MAP_LIKE_QUALIFIED_TYPE absl::flat_hash_map
		#elif LRU_CACHE_HASH_MAP_IMPLEMENTATION == TESSIL_SPARSE_MAP
			#define STL_UNORDERED_MAP_LIKE_QUALIFIED_TYPE tsl::sparse_map
		#elif LRU_CACHE_HASH_MAP_IMPLEMENTATION == TESSIL_ROBIN_MAP
			#define STL_UNORDERED_MAP_LIKE_QUALIFIED_TYPE tsl::robin_map
		#elif LRU_CACHE_HASH_MAP_IMPLEMENTATION == TESSIL_HOPSCOTCH_MAP
			#define STL_UNORDERED_MAP_LIKE_QUALIFIED_TYPE tsl::hopscotch_map
		#elif LRU_CACHE_HASH_MAP_IMPLEMENTATION == ANKERL_UNORDERED_DENSE_MAP
			#define STL_UNORDERED_MAP_LIKE_QUALIFIED_TYPE ankerl::unordered_dense::map
		#elif LRU_CACHE_HASH_MAP_IMPLEMENTATION == ANKERL_UNORDERED_DENSE_SEGMENTED_MAP
			#define STL_UNORDERED_MAP_LIKE_QUALIFIED_TYPE ankerl::unordered_dense::segmented_map
		#endif

		template<typename kt, typename vt>
		using map_t = typename std::conditional_t<
			std::is_same_v<hash_function, DefaultHashFunction<kt>>,
			STL_UNORDERED_MAP_LIKE_QUALIFIED_TYPE<kt, vt>,
			typename std::conditional_t<
				std::is_same_v<key_equality_predicate, DefaultKeyEqualityPredicate<kt>>,
				STL_UNORDERED_MAP_LIKE_QUALIFIED_TYPE<kt, vt, hash_function>,
				STL_UNORDERED_MAP_LIKE_QUALIFIED_TYPE<kt, vt, hash_function, key_equality_predicate>
			>
		>;

		#undef STL_UNORDERED_MAP_LIKE_QUALIFIED_TYPE
		#ifdef STL_UNORDERED_MAP_LIKE_QUALIFIED_TYPE_BEFORE
			#define STL_UNORDERED_MAP_LIKE_QUALIFIED_TYPE STL_UNORDERED_MAP_LIKE_QUALIFIED_TYPE_BEFORE
			#undef STL_UNORDERED_MAP_LIKE_QUALIFIED_TYPE_BEFORE
		#endif

	protected:
		using key_value_pair_t = std::pair<key_t, value_t>;
		using list_index_t = typename vector_list<key_value_pair_t, max_size>::index_t;
		using map_iterator_t = typename map_t<key_t, list_index_t>::iterator;

		static constexpr bool map_has_emplace_hint = has_emplace_hint_v<map_t<key_t, list_index_t>>;
		static constexpr bool map_has_insert_with_hint = has_insert_with_hint_v<map_t<key_t, list_index_t>>;

	public:
		using const_iterator = typename vector_list<key_value_pair_t, max_size>::const_iterator;
		using const_reverse_iterator = typename vector_list<key_value_pair_t, max_size>::const_reverse_iterator;
		using iterator = typename vector_list<key_value_pair_t, max_size>::const_iterator;
		using reverse_iterator = typename vector_list<key_value_pair_t, max_size>::const_reverse_iterator;

	protected:
		vector_list<key_value_pair_t, max_size> _cache_items_list{};

		#if LRU_CACHE_HASH_MAP_IMPLEMENTATION == STL_PMR_UNORDERED_MAP
			static constexpr std::size_t map_node_overhead_estimate = 24;
			std::pmr::unsynchronized_pool_resource _unsynchronized_pool_resource{
				std::pmr::pool_options{
					/* max_blocks_per_chunk */ 64,
					/* largest_required_pool_block */ sizeof(key_value_pair_t) + map_node_overhead_estimate
				},
				std::pmr::get_default_resource()
			};
			map_t<key_t, list_index_t> _cache_items_map{&_unsynchronized_pool_resource};
		#else
			map_t<key_t, list_index_t> _cache_items_map{};
		#endif
	};

	// The base class of lru_cache that defines the default constructors, destructor and assignments.
	template<typename key_t, typename value_t, const std::size_t max_size, const bool preallocate = false, typename hash_function = DefaultHashFunction<key_t>, typename key_equality_predicate = DefaultKeyEqualityPredicate<key_t>>
	class lru_cache_base : protected lru_cache_storage_base<key_t, value_t, max_size, hash_function, key_equality_predicate> {
	public:
		lru_cache_base() = default;
		~lru_cache_base() = default;
		lru_cache_base(const lru_cache_base&) = default;
		lru_cache_base(lru_cache_base&&) = default;
		lru_cache_base& operator=(lru_cache_base const&) = default;
		lru_cache_base& operator=(lru_cache_base &&) = default;
	};

	// The base class of lru_cache that defines the custom empty constructor and other default constructors, destructor and assignments.
	template<typename key_t, typename value_t, const std::size_t max_size, typename hash_function, typename key_equality_predicate>
	class lru_cache_base<key_t, value_t, max_size, true, hash_function, key_equality_predicate> : protected lru_cache_storage_base<key_t, value_t, max_size, hash_function, key_equality_predicate> {
	public:
		using key_value_pair_t = typename lru_cache_storage_base<key_t, value_t, max_size, hash_function, key_equality_predicate>::key_value_pair_t;

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
} // guiorgy::detail

/* Restore old definitions if they were already defined */
// STL std::unordered_map
#ifdef GUIORGY_LRU_CACHE_STL_UNORDERED_MAP_BEFORE
	#undef STL_UNORDERED_MAP
	#define STL_UNORDERED_MAP GUIORGY_LRU_CACHE_STL_UNORDERED_MAP_BEFORE
	#undef GUIORGY_LRU_CACHE_STL_UNORDERED_MAP_BEFORE
#endif
// STL std::pmr::unordered_map with std::pmr::unsynchronized_pool_resource
#ifdef GUIORGY_LRU_CACHE_STL_PMR_UNORDERED_MAP_BEFORE
	#undef STL_MPR_UNORDERED_MAP
	#define STL_MPR_UNORDERED_MAP GUIORGY_LRU_CACHE_STL_PMR_UNORDERED_MAP_BEFORE
	#undef GUIORGY_LRU_CACHE_STL_PMR_UNORDERED_MAP_BEFORE
#endif

// STL std::unordered_map
#ifdef GUIORGY_LRU_CACHE_ABSEIL_FLAT_HASH_MAP_BEFORE
	#undef ABSEIL_FLAT_HASH_MAP
	#define ABSEIL_FLAT_HASH_MAP GUIORGY_LRU_CACHE_ABSEIL_FLAT_HASH_MAP_BEFORE
	#undef GUIORGY_LRU_CACHE_ABSEIL_FLAT_HASH_MAP_BEFORE
#endif

// Tessil tsl::sparse_map
#ifdef GUIORGY_LRU_CACHE_TESSIL_SPARSE_MAP_BEFORE
	#undef TESSIL_SPARSE_MAP
	#define TESSIL_SPARSE_MAP GUIORGY_LRU_CACHE_TESSIL_SPARSE_MAP_BEFORE
	#undef GUIORGY_LRU_CACHE_TESSIL_SPARSE_MAP_BEFORE
#endif
// Tessil tsl::robin_map
#ifdef GUIORGY_LRU_CACHE_TESSIL_ROBIN_MAP_BEFORE
	#undef TESSIL_ROBIN_MAP
	#define TESSIL_ROBIN_MAP GUIORGY_LRU_CACHE_TESSIL_ROBIN_MAP_BEFORE
	#undef GUIORGY_LRU_CACHE_TESSIL_ROBIN_MAP_BEFORE
#endif
// Tessil tsl::hopscotch_map
#ifdef GUIORGY_LRU_CACHE_TESSIL_HOPSCOTCH_MAP_BEFORE
	#undef TESSIL_HOPSCOTCH_MAP
	#define TESSIL_HOPSCOTCH_MAP GUIORGY_LRU_CACHE_TESSIL_HOPSCOTCH_MAP_BEFORE
	#undef GUIORGY_LRU_CACHE_TESSIL_HOPSCOTCH_MAP_BEFORE
#endif

// Ankerl ankerl::unordered_dense
#ifdef GUIORGY_LRU_CACHE_ANKERL_UNORDERED_DENSE_MAP_BEFORE
	#undef ANKERL_UNORDERED_DENSE_MAP
	#define ANKERL_UNORDERED_DENSE_MAP GUIORGY_LRU_CACHE_ANKERL_UNORDERED_DENSE_MAP_BEFORE
	#undef GUIORGY_LRU_CACHE_ANKERL_UNORDERED_DENSE_MAP_BEFORE
#endif
// Ankerl ankerl::unordered_dense::segmented_map
#ifdef GUIORGY_LRU_CACHE_ANKERL_UNORDERED_DENSE_SEGMENTED_MAP_BEFORE
	#undef ANKERL_UNORDERED_DENSE_SEGMENTED_MAP
	#define ANKERL_UNORDERED_DENSE_SEGMENTED_MAP GUIORGY_LRU_CACHE_ANKERL_UNORDERED_DENSE_SEGMENTED_MAP_BEFORE
	#undef GUIORGY_LRU_CACHE_ANKERL_UNORDERED_DENSE_SEGMENTED_MAP_BEFORE
#endif

// lru_cache public.
namespace guiorgy {
	// A hint to the likelihood of a condition being true.
	// Remarks:
	//   - This has no effect in C++ < 20.
	enum struct Likelihood : std::uint_fast8_t {
		Unknown,	// No attribute used
		Likely,		// [[likely]] attribute used
		Unlikely	// [[unlikely]] attribute used
	};

	// A fixed size (if preallocate is true) or bounded (if preallocate is false) container that
	// contains key-value pairs with unique keys. Search, insertion, and removal of elements have
	// average constant-time complexity. Two keys are considered equivalent if key_equality_predicate
	// predicate returns true when passed those keys. If two keys are equivalent, the hash_function
	// hash function must return the same value for both keys. lru_cache can not be created and
	// used in the evaluation of a constant expression (constexpr).
	// When filled, the container uses the Least Recently Used replacement policy to store subsequent elements.
	// Remarks:
	//   - The inserted values and their members don't have stable memory addresses as they may be copied/moved when the cache grows.
	//     If that is required, or values are too expensive to copy/move, either use std::unique_ptr<value_t> or something similar
	//     as the value type instead, or tell lru_cache to preallocate the needed storage by using reserve().
	//   - The removed elements are not deleted immediately, instead they are replaced when new elements are put into the container.
	//   - When using std::unordered_map:
	//     - lru_cache is default constructible.
	//     - lru_cache is not trivially (default) constructible.
	//     - lru_cache is nothrow (default) constructible only if preallocate is false.
	//     - lru_cache is copy constructible.
	//     - lru_cache is not trivially copy constructible.
	//     - lru_cache is not nothrow copy constructible.
	//     - lru_cache is move constructible.
	//     - lru_cache is not trivially move constructible.
	//     - lru_cache is nothrow move constructible.
	//   - When using std::pmr::unordered_map:
	//     - lru_cache is default constructible.
	//     - lru_cache is not trivially (default) constructible.
	//     - lru_cache is not nothrow (default) constructible.
	//     - lru_cache is not copy constructible.
	//     - lru_cache is not trivially copy constructible.
	//     - lru_cache is not nothrow copy constructible.
	//     - lru_cache is not move constructible.
	//     - lru_cache is not trivially move constructible.
	//     - lru_cache is not nothrow move constructible.
	//   - When using absl::flat_hash_map, tsl::sparse_map, tsl::robin_map, tsl::hopscotch_map, ankerl::unordered_dense::map or ankerl::unordered_dense::segmented_map:
	//     - lru_cache is default constructible.
	//     - lru_cache is not trivially (default) constructible.
	//     - lru_cache is not nothrow (default) constructible.
	//     - lru_cache is copy constructible.
	//     - lru_cache is not trivially copy constructible.
	//     - lru_cache is not nothrow copy constructible.
	//     - lru_cache is move constructible.
	//     - lru_cache is not trivially move constructible.
	//     - lru_cache is nothrow move constructible.
	template<typename key_t, typename value_t, const std::size_t max_size, const bool preallocate = false, typename hash_function = detail::DefaultHashFunction<key_t>, typename key_equality_predicate = detail::DefaultKeyEqualityPredicate<key_t>>
	class lru_cache final : private detail::lru_cache_base<key_t, value_t, max_size, preallocate, hash_function, key_equality_predicate> {
		static_assert(max_size != 0u, "max_size can not be 0");

		using key_value_pair_t = typename detail::lru_cache_storage_base<key_t, value_t, max_size, hash_function, key_equality_predicate>::key_value_pair_t;
		using list_index_t = typename detail::lru_cache_storage_base<key_t, value_t, max_size, hash_function, key_equality_predicate>::list_index_t;
		using map_iterator_t = typename detail::lru_cache_storage_base<key_t, value_t, max_size, hash_function, key_equality_predicate>::map_iterator_t;

		// Checks whether the given likelihood value is a valid Likelihood enum value.
		static constexpr bool is_valid_likelihood(const Likelihood likelihood) noexcept {
			return likelihood == Likelihood::Unknown || likelihood == Likelihood::Likely || likelihood == Likelihood::Unlikely;
		}

		// Converts a bool to a Likelihood::Likely or Likelihood::Unlikely enum.
		// The Likelihood::Unknown value is impossible to get this way.
		static constexpr Likelihood likelihood(const bool likely) noexcept {
			return likely ? Likelihood::Likely : Likelihood::Unlikely;
		}

	public:
		using iterator = typename detail::lru_cache_storage_base<key_t, value_t, max_size, hash_function, key_equality_predicate>::iterator;
		using reverse_iterator = typename detail::lru_cache_storage_base<key_t, value_t, max_size, hash_function, key_equality_predicate>::reverse_iterator;
		using const_iterator = typename detail::lru_cache_storage_base<key_t, value_t, max_size, hash_function, key_equality_predicate>::const_iterator;
		using const_reverse_iterator = typename detail::lru_cache_storage_base<key_t, value_t, max_size, hash_function, key_equality_predicate>::const_reverse_iterator;

		// If the key already exists in the container, copies value to the mapped value inside the container.
		// If the key doesn't exist in the container, inserts a copy of value into the container.
		// If a reallocation takes place after the operation, or the container size is already at max_size,
		// all references and iterators are invalidated.
		// Use key_exists and cache_full to hint to the compiler for which case to optimize for.
		template<const Likelihood key_exists = Likelihood::Unknown, const Likelihood cache_full = Likelihood::Unknown>
		void put(const key_t& key, const value_t& value) {
			map_iterator_t it = this->_cache_items_map.find(key);

			static_assert(is_valid_likelihood(key_exists), "key_exists has an invalid enum value for Likelihood");
			if constexpr (key_exists == Likelihood::Unknown) {
				if (it != this->_cache_items_map.end()) {
					this->_cache_items_list._get_value_at(it->second).second = value;
					this->_cache_items_list._move_value_at_to_front(it->second);

					return;
				}
			} else if constexpr (key_exists == Likelihood::Likely) {
				if (it != this->_cache_items_map.end()) LIKELY {
					this->_cache_items_list._get_value_at(it->second).second = value;
					this->_cache_items_list._move_value_at_to_front(it->second);

					return;
				}
			} else if constexpr (key_exists == Likelihood::Unlikely) {
				if (it != this->_cache_items_map.end()) UNLIKELY {
					this->_cache_items_list._get_value_at(it->second).second = value;
					this->_cache_items_list._move_value_at_to_front(it->second);

					return;
				}
			}

			static_assert(is_valid_likelihood(cache_full), "cache_full has an invalid enum value for Likelihood");
			if constexpr (cache_full == Likelihood::Unknown) {
				if (this->_cache_items_map.size() == max_size) {
					key_value_pair_t& last = this->_cache_items_list.back();
					this->_cache_items_map.erase(last.first);
					last.first = key;
					last.second = value;
					this->_cache_items_list._move_last_value_to_front();
				} else {
					this->_cache_items_list.emplace_front(key, value);
				}
			} else if constexpr (cache_full == Likelihood::Likely) {
				if (this->_cache_items_map.size() == max_size) LIKELY  {
					key_value_pair_t& last = this->_cache_items_list.back();
					this->_cache_items_map.erase(last.first);
					last.first = key;
					last.second = value;
					this->_cache_items_list._move_last_value_to_front();
				} else {
					this->_cache_items_list.emplace_front(key, value);
				}
			} else if constexpr (cache_full == Likelihood::Unlikely) {
				if (this->_cache_items_map.size() == max_size) UNLIKELY {
					key_value_pair_t& last = this->_cache_items_list.back();
					this->_cache_items_map.erase(last.first);
					last.first = key;
					last.second = value;
					this->_cache_items_list._move_last_value_to_front();
				} else {
					this->_cache_items_list.emplace_front(key, value);
				}
			}

			if constexpr (this->map_has_emplace_hint) {
				this->_cache_items_map.emplace_hint(it, key, this->_cache_items_list._first_value_index());
			} else if constexpr (this->map_has_insert_with_hint) {
				this->_cache_items_map.insert(it, std::make_pair(key, this->_cache_items_list._first_value_index()));
			} else {
				this->_cache_items_map[key] = this->_cache_items_list._first_value_index();
			}
		}

		// See the put above for details.
		// Use key_likely_exists and cache_likely_full to hint to the compiler for which case to optimize for.
		template<const bool key_likely_exists, const bool cache_likely_full>
		void put(const key_t& key, const value_t& value) {
			put<likelihood(key_likely_exists), likelihood(cache_likely_full)>(key, value);
		}
		template<const bool key_likely_exists>
		void put(const key_t& key, const value_t& value) {
			put<likelihood(key_likely_exists)>(key, value);
		}

		// If the key already exists in the container, moves value to the mapped value inside the container.
		// If the key doesn't exist in the container, inserts value into the container.
		// If a reallocation takes place after the operation, or the container size is already at max_size,
		// all references and iterators are invalidated.
		// Use key_exists and cache_full to hint to the compiler for which case to optimize for.
		template<const Likelihood key_exists = Likelihood::Unknown, const Likelihood cache_full = Likelihood::Unknown>
		void put(const key_t& key, value_t&& value) {
			map_iterator_t it = this->_cache_items_map.find(key);

			static_assert(is_valid_likelihood(key_exists), "key_exists has an invalid enum value for Likelihood");
			if constexpr (key_exists == Likelihood::Unknown) {
				if (it != this->_cache_items_map.end()) {
					this->_cache_items_list._get_value_at(it->second).second = std::move(value);
					this->_cache_items_list._move_value_at_to_front(it->second);

					return;
				}
			} else if constexpr (key_exists == Likelihood::Likely) {
				if (it != this->_cache_items_map.end()) LIKELY {
					this->_cache_items_list._get_value_at(it->second).second = std::move(value);
					this->_cache_items_list._move_value_at_to_front(it->second);

					return;
				}
			} else if constexpr (key_exists == Likelihood::Unlikely) {
				if (it != this->_cache_items_map.end()) UNLIKELY {
					this->_cache_items_list._get_value_at(it->second).second = std::move(value);
					this->_cache_items_list._move_value_at_to_front(it->second);

					return;
				}
			}

			static_assert(is_valid_likelihood(cache_full), "cache_full has an invalid enum value for Likelihood");
			if constexpr (cache_full == Likelihood::Unknown) {
				if (this->_cache_items_map.size() == max_size) {
					key_value_pair_t& last = this->_cache_items_list.back();
					this->_cache_items_map.erase(last.first);
					last.first = key;
					last.second = std::move(value);
					this->_cache_items_list._move_last_value_to_front();
				} else {
					this->_cache_items_list.emplace_front(key, std::move(value));
				}
			} else if constexpr (cache_full == Likelihood::Likely) {
				if (this->_cache_items_map.size() == max_size) LIKELY  {
					key_value_pair_t& last = this->_cache_items_list.back();
					this->_cache_items_map.erase(last.first);
					last.first = key;
					last.second = std::move(value);
					this->_cache_items_list._move_last_value_to_front();
				} else {
					this->_cache_items_list.emplace_front(key, std::move(value));
				}
			} else if constexpr (cache_full == Likelihood::Unlikely) {
				if (this->_cache_items_map.size() == max_size) UNLIKELY {
					key_value_pair_t& last = this->_cache_items_list.back();
					this->_cache_items_map.erase(last.first);
					last.first = key;
					last.second = std::move(value);
					this->_cache_items_list._move_last_value_to_front();
				} else {
					this->_cache_items_list.emplace_front(key, std::move(value));
				}
			}

			if constexpr (this->map_has_emplace_hint) {
				this->_cache_items_map.emplace_hint(it, key, this->_cache_items_list._first_value_index());
			} else if constexpr (this->map_has_insert_with_hint) {
				this->_cache_items_map.insert(it, std::make_pair(key, this->_cache_items_list._first_value_index()));
			} else {
				this->_cache_items_map[key] = this->_cache_items_list._first_value_index();
			}
		}

		// See the put above for details.
		// Use key_likely_exists and cache_likely_full to hint to the compiler for which case to optimize for.
		template<const bool key_likely_exists, const bool cache_likely_full>
		void put(const key_t& key, value_t&& value) {
			put<likelihood(key_likely_exists), likelihood(cache_likely_full)>(key, std::move(value));
		}
		template<const bool key_likely_exists>
		void put(const key_t& key, value_t&& value) {
			put<likelihood(key_likely_exists)>(key, std::move(value));
		}

		// If the key already exists in the container, constructs a new element in-place with the given value_args into the mapped value inside the container.
		// If the key doesn't exist in the container, constructs a new element in-place with the given value_args into the container.
		// If a reallocation takes place after the operation, or the container size is already at max_size,
		// all references and iterators are invalidated.
		// Use key_exists and cache_full to hint to the compiler for which case to optimize for.
		template<const Likelihood key_exists = Likelihood::Unknown, const Likelihood cache_full = Likelihood::Unknown, typename... ValueArgs>
		const value_t& emplace(const key_t& key, ValueArgs&&... value_args) {
			map_iterator_t it = this->_cache_items_map.find(key);

			static_assert(is_valid_likelihood(key_exists), "key_exists has an invalid enum value for Likelihood");
			if constexpr (key_exists == Likelihood::Unknown) {
				if (it != this->_cache_items_map.end()) {
					value_t& value = this->_cache_items_list._get_value_at(it->second).second;
					detail::emplace(&value, std::forward<ValueArgs>(value_args)...);
					this->_cache_items_list._move_value_at_to_front(it->second);

					return value;
				}
			} else if constexpr (key_exists == Likelihood::Likely) {
				if (it != this->_cache_items_map.end()) LIKELY {
					value_t& value = this->_cache_items_list._get_value_at(it->second).second;
					detail::emplace(&value, std::forward<ValueArgs>(value_args)...);
					this->_cache_items_list._move_value_at_to_front(it->second);

					return value;
				}
			} else if constexpr (key_exists == Likelihood::Unlikely) {
				if (it != this->_cache_items_map.end()) UNLIKELY {
					value_t& value = this->_cache_items_list._get_value_at(it->second).second;
					detail::emplace(&value, std::forward<ValueArgs>(value_args)...);
					this->_cache_items_list._move_value_at_to_front(it->second);

					return value;
				}
			}

			value_t* value;

			static_assert(is_valid_likelihood(cache_full), "cache_full has an invalid enum value for Likelihood");
			if constexpr (cache_full == Likelihood::Unknown) {
				if (this->_cache_items_map.size() == max_size) {
					key_value_pair_t& last = this->_cache_items_list.back();
					this->_cache_items_map.erase(last.first);
					last.first = key;
					detail::emplace(&last.second, std::forward<ValueArgs>(value_args)...);
					this->_cache_items_list._move_last_value_to_front();

					value = &last.second;
				} else {
					value = &(this->_cache_items_list.emplace_front(key, std::forward<ValueArgs>(value_args)...));
				}
			} else if constexpr (cache_full == Likelihood::Likely) {
				if (this->_cache_items_map.size() == max_size) LIKELY  {
					key_value_pair_t& last = this->_cache_items_list.back();
					this->_cache_items_map.erase(last.first);
					last.first = key;
					detail::emplace(&last.second, std::forward<ValueArgs>(value_args)...);
					this->_cache_items_list._move_last_value_to_front();

					value = &last.second;
				} else {
					value = &(this->_cache_items_list.emplace_front(key, std::forward<ValueArgs>(value_args)...));
				}
			} else if constexpr (cache_full == Likelihood::Unlikely) {
				if (this->_cache_items_map.size() == max_size) UNLIKELY {
					key_value_pair_t& last = this->_cache_items_list.back();
					this->_cache_items_map.erase(last.first);
					last.first = key;
					detail::emplace(&last.second, std::forward<ValueArgs>(value_args)...);
					this->_cache_items_list._move_last_value_to_front();

					value = &last.second;
				} else {
					value = &(this->_cache_items_list.emplace_front(key, std::forward<ValueArgs>(value_args)...));
				}
			}

			if constexpr (this->map_has_emplace_hint) {
				this->_cache_items_map.emplace_hint(it, key, this->_cache_items_list._first_value_index());
			} else if constexpr (this->map_has_insert_with_hint) {
				this->_cache_items_map.insert(it, std::make_pair(key, this->_cache_items_list._first_value_index()));
			} else {
				this->_cache_items_map[key] = this->_cache_items_list._first_value_index();
			}

			return *value;
		}

		// See the emplace above for details.
		// Use key_likely_exists and cache_likely_full to hint to the compiler for which case to optimize for.
		template<const bool key_likely_exists, const bool cache_likely_full, typename... ValueArgs>
		void emplace(const key_t& key, ValueArgs&&... value_args) {
			emplace<likelihood(key_likely_exists), likelihood(cache_likely_full)>(key, std::forward<ValueArgs>(value_args)...);
		}
		template<const bool key_likely_exists, typename... ValueArgs>
		void emplace(const key_t& key, ValueArgs&&... value_args) {
			emplace<likelihood(key_likely_exists)>(key, std::forward<ValueArgs>(value_args)...);
		}

		// Returns a std::optional with the value that is mapped to the given key,
		// or an empty optional if such key does not already exist.
		// Use key_exists to hint to the compiler for which case to optimize for.
		template<const Likelihood key_exists = Likelihood::Unknown>
		[[nodiscard]] const std::optional<value_t> get(const key_t& key) {
			map_iterator_t it = this->_cache_items_map.find(key);

			static_assert(is_valid_likelihood(key_exists), "key_exists has an invalid enum value for Likelihood");
			if constexpr (key_exists == Likelihood::Unknown) {
				if (it != this->_cache_items_map.end()) {
					return this->_cache_items_list._move_value_at_to_front(it->second).second;
				} else {
					return std::nullopt;
				}
			} else if constexpr (key_exists == Likelihood::Likely) {
				if (it != this->_cache_items_map.end()) LIKELY {
					return this->_cache_items_list._move_value_at_to_front(it->second).second;
				} else {
					return std::nullopt;
				}
			} else if constexpr (key_exists == Likelihood::Unlikely) {
				if (it != this->_cache_items_map.end()) UNLIKELY {
					return this->_cache_items_list._move_value_at_to_front(it->second).second;
				} else {
					return std::nullopt;
				}
			}
		}

		// See the get above for details.
		// Use key_likely_exists to hint to the compiler for which case to optimize for.
		template<const bool key_likely_exists>
		[[nodiscard]] const std::optional<value_t> get(const key_t& key) {
			return get<likelihood(key_likely_exists)>(key);
		}

		// Returns a std::optional with a reference to the value that is mapped to
		// the given key, or an empty optional if such key does not already exist.
		// Use key_exists to hint to the compiler for which case to optimize for.
		// Remarks:
		//   - No guarantees are given about the underlying object lifetime when
		//     modifying the cache (inserting/removing elements), so use with caution.
		template<const Likelihood key_exists = Likelihood::Unknown>
		[[nodiscard]] const std::optional<std::reference_wrapper<const value_t>> get_ref(const key_t& key) {
			map_iterator_t it = this->_cache_items_map.find(key);

			static_assert(is_valid_likelihood(key_exists), "key_exists has an invalid enum value for Likelihood");
			if constexpr (key_exists == Likelihood::Unknown) {
				if (it != this->_cache_items_map.end()) {
					return std::make_optional(std::cref(this->_cache_items_list._move_value_at_to_front(it->second).second));
				} else {
					return std::nullopt;
				}
			} else if constexpr (key_exists == Likelihood::Likely) {
				if (it != this->_cache_items_map.end()) LIKELY {
					return std::make_optional(std::cref(this->_cache_items_list._move_value_at_to_front(it->second).second));
				} else {
					return std::nullopt;
				}
			} else if constexpr (key_exists == Likelihood::Unlikely) {
				if (it != this->_cache_items_map.end()) UNLIKELY {
					return std::make_optional(std::cref(this->_cache_items_list._move_value_at_to_front(it->second).second));
				} else {
					return std::nullopt;
				}
			}
		}

		// See the get_ref above for details.
		// Use key_likely_exists to hint to the compiler for which case to optimize for.
		template<const bool key_likely_exists>
		[[nodiscard]] const std::optional<std::reference_wrapper<const value_t>> get_ref(const key_t& key) {
			return get_ref<likelihood(key_likely_exists)>(key);
		}

		// Returns true and copies the value that is mapped to the given key into
		// the given value_out reference, or false if such key does not already exist.
		// Use key_exists to hint to the compiler for which case to optimize for.
		template<const Likelihood key_exists = Likelihood::Unknown>
		[[nodiscard]] bool try_get(const key_t& key, value_t& value_out) {
			map_iterator_t it = this->_cache_items_map.find(key);

			static_assert(is_valid_likelihood(key_exists), "key_exists has an invalid enum value for Likelihood");
			if constexpr (key_exists == Likelihood::Unknown) {
				if (it != this->_cache_items_map.end()) {
					value_out = this->_cache_items_list._move_value_at_to_front(it->second).second;
					return true;
				} else {
					return false;
				}
			} else if constexpr (key_exists == Likelihood::Likely) {
				if (it != this->_cache_items_map.end()) LIKELY {
					value_out = this->_cache_items_list._move_value_at_to_front(it->second).second;
					return true;
				} else {
					return false;
				}
			} else if constexpr (key_exists == Likelihood::Unlikely) {
				if (it != this->_cache_items_map.end()) UNLIKELY {
					value_out = this->_cache_items_list._move_value_at_to_front(it->second).second;
					return true;
				} else {
					return false;
				}
			}
		}

		// See the try_get above for details.
		// Use key_likely_exists to hint to the compiler for which case to optimize for.
		template<const bool key_likely_exists>
		[[nodiscard]] bool try_get(const key_t& key, value_t& value_out) {
			return try_get<likelihood(key_likely_exists)>(key, value_out);
		}

		// Returns true and assigns the address to the value that is mapped to the
		// given key into the given value_out pointer reference, or false if such
		// key does not already exist.
		// Use key_exists to hint to the compiler for which case to optimize for.
		// Remarks:
		//   - No guarantees are given about the underlying object lifetime when
		//     modifying the cache (inserting/removing elements), so use with caution.
		template<const Likelihood key_exists = Likelihood::Unknown>
		[[nodiscard]] bool try_get_ref(const key_t& key, const value_t*& value_out) {
			map_iterator_t it = this->_cache_items_map.find(key);

			static_assert(is_valid_likelihood(key_exists), "key_exists has an invalid enum value for Likelihood");
			if constexpr (key_exists == Likelihood::Unknown) {
				if (it != this->_cache_items_map.end()) {
					value_out = &(this->_cache_items_list._move_value_at_to_front(it->second).second);
					return true;
				} else {
					return false;
				}
			} else if constexpr (key_exists == Likelihood::Likely) {
				if (it != this->_cache_items_map.end()) LIKELY {
					value_out = &(this->_cache_items_list._move_value_at_to_front(it->second).second);
					return true;
				} else {
					return false;
				}
			} else if constexpr (key_exists == Likelihood::Unlikely) {
				if (it != this->_cache_items_map.end()) UNLIKELY {
					value_out = &(this->_cache_items_list._move_value_at_to_front(it->second).second);
					return true;
				} else {
					return false;
				}
			}
		}

		// See the try_get_ref above for details.
		// Use key_likely_exists to hint to the compiler for which case to optimize for.
		template<const bool key_likely_exists>
		[[nodiscard]] bool try_get_ref(const key_t& key, const value_t*& value_out) {
			return try_get_ref<likelihood(key_likely_exists)>(key, value_out);
		}

		// If the key exists in the container, removes the value that is mapped to the
		// given key and returns a std::optional with the removed value,
		// otherwise, returns an empty optional.
		// Use key_exists to hint to the compiler for which case to optimize for.
		template<const Likelihood key_exists = Likelihood::Unknown>
		[[nodiscard("Use erase(const key_t& key) instead")]]
		std::optional<value_t> remove(const key_t& key) {
			map_iterator_t it = this->_cache_items_map.find(key);

			static_assert(is_valid_likelihood(key_exists), "key_exists has an invalid enum value for Likelihood");
			if constexpr (key_exists == Likelihood::Unknown) {
				if (it != this->_cache_items_map.end()) {
					value_t& value = this->_cache_items_list._erase_value_at(it->second).second;
					this->_cache_items_map.erase(it);
					return value;
				} else {
					return std::nullopt;
				}
			} else if constexpr (key_exists == Likelihood::Likely) {
				if (it != this->_cache_items_map.end()) LIKELY {
					value_t& value = this->_cache_items_list._erase_value_at(it->second).second;
					this->_cache_items_map.erase(it);
					return value;
				} else {
					return std::nullopt;
				}
			} else if constexpr (key_exists == Likelihood::Unlikely) {
				if (it != this->_cache_items_map.end()) UNLIKELY {
					value_t& value = this->_cache_items_list._erase_value_at(it->second).second;
					this->_cache_items_map.erase(it);
					return value;
				} else {
					return std::nullopt;
				}
			}
		}

		// See the remove above for details.
		// Use key_likely_exists to hint to the compiler for which case to optimize for.
		template<const bool key_likely_exists>
		[[nodiscard("Use erase(const key_t& key) instead")]]
		std::optional<value_t> remove(const key_t& key) {
			return remove<likelihood(key_likely_exists)>(key);
		}

		// If the key exists in the container, removes the value that is mapped to the
		// given key and returns a std::optional with a reference to the removed value,
		// otherwise, returns an empty optional.
		// Use key_exists to hint to the compiler for which case to optimize for.
		// Remarks:
		//   - No guarantees are given about the underlying object lifetime when
		//     modifying the cache (inserting/removing elements), so use with caution.
		template<const Likelihood key_exists = Likelihood::Unknown>
		[[nodiscard("Use erase(const key_t& key) instead")]]
		std::optional<std::reference_wrapper<value_t>> remove_ref(const key_t& key) {
			map_iterator_t it = this->_cache_items_map.find(key);

			static_assert(is_valid_likelihood(key_exists), "key_exists has an invalid enum value for Likelihood");
			if constexpr (key_exists == Likelihood::Unknown) {
				if (it != this->_cache_items_map.end()) {
					value_t& value = this->_cache_items_list._erase_value_at(it->second).second;
					this->_cache_items_map.erase(it);
					return std::make_optional(std::ref(value));
				} else {
					return std::nullopt;
				}
			} else if constexpr (key_exists == Likelihood::Likely) {
				if (it != this->_cache_items_map.end()) LIKELY {
					value_t& value = this->_cache_items_list._erase_value_at(it->second).second;
					this->_cache_items_map.erase(it);
					return std::make_optional(std::ref(value));
				} else {
					return std::nullopt;
				}
			} else if constexpr (key_exists == Likelihood::Unlikely) {
				if (it != this->_cache_items_map.end()) UNLIKELY {
					value_t& value = this->_cache_items_list._erase_value_at(it->second).second;
					this->_cache_items_map.erase(it);
					return std::make_optional(std::ref(value));
				} else {
					return std::nullopt;
				}
			}
		}

		// See the remove_ref above for details.
		// Use key_likely_exists to hint to the compiler for which case to optimize for.
		template<const bool key_likely_exists>
		[[nodiscard("Use erase(const key_t& key) instead")]]
		std::optional<std::reference_wrapper<value_t>> remove_ref(const key_t& key) {
			return remove_ref<likelihood(key_likely_exists)>(key);
		}

		// If the key exists in the container, removes the value that is mapped to the
		// given key, returns true and copies the value that is mapped to the given key
		// into the given value_out reference, or false if such key does not exist.
		// Use key_exists to hint to the compiler for which case to optimize for.
		template<const Likelihood key_exists = Likelihood::Unknown>
		[[nodiscard("Use erase(const key_t& key) instead")]]
		bool try_remove(const key_t& key, value_t& value_out) {
			map_iterator_t it = this->_cache_items_map.find(key);

			static_assert(is_valid_likelihood(key_exists), "key_exists has an invalid enum value for Likelihood");
			if constexpr (key_exists == Likelihood::Unknown) {
				if (it != this->_cache_items_map.end()) {
					value_out = this->_cache_items_list._erase_value_at(it->second).second;
					this->_cache_items_map.erase(it);
					return true;
				} else {
					return false;
				}
			} else if constexpr (key_exists == Likelihood::Likely) {
				if (it != this->_cache_items_map.end()) LIKELY {
					value_out = this->_cache_items_list._erase_value_at(it->second).second;
					this->_cache_items_map.erase(it);
					return true;
				} else {
					return false;
				}
			} else if constexpr (key_exists == Likelihood::Unlikely) {
				if (it != this->_cache_items_map.end()) UNLIKELY {
					value_out = this->_cache_items_list._erase_value_at(it->second).second;
					this->_cache_items_map.erase(it);
					return true;
				} else {
					return false;
				}
			}
		}

		// See the try_remove above for details.
		// Use key_likely_exists to hint to the compiler for which case to optimize for.
		template<const bool key_likely_exists>
		[[nodiscard("Use erase(const key_t& key) instead")]]
		bool try_remove(const key_t& key, value_t& value_out) {
			return try_remove<likelihood(key_likely_exists)>(key, value_out);
		}

		// If the key exists in the container, removes the value that is mapped to the
		// given key, returns true and assigns the address to the value that is mapped
		// to the given key into the given value_out pointer reference, or false if
		// such key does not exist.
		// Use key_exists to hint to the compiler for which case to optimize for.
		// Remarks:
		//   - No guarantees are given about the underlying object lifetime when
		//     modifying the cache (inserting/removing elements), so use with caution.
		template<const Likelihood key_exists = Likelihood::Unknown>
		[[nodiscard("Use erase(const key_t& key) instead")]]
		bool try_remove_ref(const key_t& key, const value_t*& value_out) {
			map_iterator_t it = this->_cache_items_map.find(key);

			static_assert(is_valid_likelihood(key_exists), "key_exists has an invalid enum value for Likelihood");
			if constexpr (key_exists == Likelihood::Unknown) {
				if (it != this->_cache_items_map.end()) {
					value_out = &(this->_cache_items_list._erase_value_at(it->second).second);
					this->_cache_items_map.erase(it);
					return true;
				} else {
					return false;
				}
			} else if constexpr (key_exists == Likelihood::Likely) {
				if (it != this->_cache_items_map.end()) LIKELY {
					value_out = &(this->_cache_items_list._erase_value_at(it->second).second);
					this->_cache_items_map.erase(it);
					return true;
				} else {
					return false;
				}
			} else if constexpr (key_exists == Likelihood::Unlikely) {
				if (it != this->_cache_items_map.end()) UNLIKELY {
					value_out = &(this->_cache_items_list._erase_value_at(it->second).second);
					this->_cache_items_map.erase(it);
					return true;
				} else {
					return false;
				}
			}
		}

		// See the try_remove_ref above for details.
		// Use key_likely_exists to hint to the compiler for which case to optimize for.
		template<const bool key_likely_exists>
		[[nodiscard("Use erase(const key_t& key) instead")]]
		bool try_remove_ref(const key_t& key, const value_t*& value_out) {
			return try_remove_ref<likelihood(key_likely_exists)>(key, value_out);
		}

		// If the key exists in the container, removes the value that is mapped to the
		// given key and returns true, or false if such key does not exist.
		// Use key_exists to hint to the compiler for which case to optimize for.
		template<const Likelihood key_exists = Likelihood::Unknown>
		bool erase(const key_t& key) {
			map_iterator_t it = this->_cache_items_map.find(key);

			static_assert(is_valid_likelihood(key_exists), "key_exists has an invalid enum value for Likelihood");
			if constexpr (key_exists == Likelihood::Unknown) {
				if (it != this->_cache_items_map.end()) {
					this->_cache_items_list._erase_value_at(it->second);
					this->_cache_items_map.erase(it);
					return true;
				} else {
					return false;
				}
			} else if constexpr (key_exists == Likelihood::Likely) {
				if (it != this->_cache_items_map.end()) LIKELY {
					this->_cache_items_list._erase_value_at(it->second);
					this->_cache_items_map.erase(it);
					return true;
				} else {
					return false;
				}
			} else if constexpr (key_exists == Likelihood::Unlikely) {
				if (it != this->_cache_items_map.end()) UNLIKELY {
					this->_cache_items_list._erase_value_at(it->second);
					this->_cache_items_map.erase(it);
					return true;
				} else {
					return false;
				}
			}
		}

		// See the erase above for details.
		// Use key_likely_exists to hint to the compiler for which case to optimize for.
		template<const bool key_likely_exists>
		bool erase(const key_t& key) {
			return erase<likelihood(key_likely_exists)>(key);
		}

		// Checks if the container contains an element with the given key.
		[[nodiscard]] bool exists(const key_t& key) const {
			return this->_cache_items_map.find(key) != this->_cache_items_map.end();
		}

		// Returns the number of elements in the container.
		[[nodiscard]] std::size_t size() const noexcept {
			return this->_cache_items_map.size();
		}

		// Erases all elements from the container. After this call, size() returns zero.
		// References referring to contained elements are not invalidated, since the elements are deleted lazily.
		// Invalidates any iterators referring to contained elements.
		void clear() noexcept {
			this->_cache_items_map.clear();
			this->_cache_items_list.clear();
		}

		// Preallocates memory for at least max_size number of elements.
		// Remarks:
		//   - Some hashmap implementations (e.g. std::unordered_map) only reserve capacity for the buckets.
		//     In that case, element insertions will still cause some allocations.
		void reserve() {
			this->_cache_items_map.reserve(max_size);
			this->_cache_items_list.reserve(max_size);
		}

		// Returns a const iterator to the first (most recently used) element of the container.
		// Remarks:
		//   - Accessing elements through iterators does not change their order of replacement.
		[[nodiscard]] const_iterator cbegin() const noexcept {
			return this->_cache_items_list.cbegin();
		}

		// Returns a const iterator to the first (most recently used) element of the container.
		// Remarks:
		//   - Equivalent to cbegin().
		//   - Accessing elements through iterators does not change their order of replacement.
		[[nodiscard]] iterator begin() const noexcept {
			return cbegin();
		}

		// Returns a const iterator to an invalid element of the container.
		// This returned iterator only acts as a sentinel. It is not guaranteed to be dereferenceable.
		// Remarks:
		//   - Accessing elements through iterators does not change their order of replacement.
		[[nodiscard]] const_iterator cend() const noexcept {
			return this->_cache_items_list.cend();
		}

		// Returns a const iterator to an invalid element of the container.
		// This returned iterator only acts as a sentinel. It is not guaranteed to be dereferenceable.
		// Remarks:
		//   - Equivalent to cend().
		//   - Accessing elements through iterators does not change their order of replacement.
		[[nodiscard]] iterator end() const noexcept {
			return cend();
		}

		// Returns a const reverse iterator to the first element of the reversed container.
		// It corresponds to the last (least recently used) element of the non-reversed container.
		// Remarks:
		//   - Accessing elements through iterators does not change their order of replacement.
		[[nodiscard]] const_reverse_iterator crbegin() const noexcept {
			return this->_cache_items_list.crbegin();
		}

		// Returns a const reverse iterator to the first element of the reversed container.
		// It corresponds to the last (least recently used) element of the non-reversed container.
		// Remarks:
		//   - Equivalent to crbegin().
		//   - Accessing elements through iterators does not change their order of replacement.
		[[nodiscard]] reverse_iterator rbegin() const noexcept {
			return crbegin();
		}

		// Returns a const reverse iterator to an invalid element of the container.
		// It corresponds to the element preceding the first element of the non-reversed container.
		// This returned iterator only acts as a sentinel. It is not guaranteed to be dereferenceable.
		// Remarks:
		//   - Accessing elements through iterators does not change their order of replacement.
		[[nodiscard]] const_reverse_iterator crend() const noexcept {
			return this->_cache_items_list.crend();
		}

		// Returns a const reverse iterator to an invalid element of the container.
		// It corresponds to the element preceding the first element of the non-reversed container.
		// This returned iterator only acts as a sentinel. It is not guaranteed to be dereferenceable.
		// Remarks:
		//   - Equivalent to crbegin().
		//   - Accessing elements through iterators does not change their order of replacement.
		[[nodiscard]] reverse_iterator rend() const noexcept {
			return crend();
		}
	};
} // guiorgy

// Restore LIKELY and UNLIKELY if they were already defined.
#ifdef GUIORGY_LIKELY_BEFORE
	#undef LIKELY
	#define LIKELY GUIORGY_LIKELY_BEFORE
	#undef GUIORGY_LIKELY_BEFORE
#endif
#ifdef GUIORGY_UNLIKELY_BEFORE
	#undef UNLIKELY
	#define UNLIKELY GUIORGY_UNLIKELY_BEFORE
	#undef GUIORGY_UNLIKELY_BEFORE
#endif

// Restore nodiscard if it was already defined.
#ifdef GUIORGY_nodiscard_BEFORE
	#undef nodiscard
	#define nodiscard GUIORGY_nodiscard_BEFORE
	#undef GUIORGY_nodiscard_BEFORE
#endif
