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

// Error out if an unsupported C++ version is used.
#if __cplusplus < 201703L
	#error "C++ 17 is the minimum supported C++ verson"
#endif

#include <unordered_map>
#include <type_traits>
#include <functional>
#include <stdexcept>
#include <optional>
#include <cassert>
#include <cstdint>
#include <cstddef>
#include <utility>
#include <limits>
#include <memory>
#include <vector>

// A helper macro that determines whether a specific attribute is supported by the current compiler. If __has_cpp_attribute is not defined, __cplusplus is used as a fallback.
#ifdef __has_cpp_attribute
	#define GUIORGY_ATTRIBUTE_AVAILABLE(attribute_token, test_value, cpp_version) \
		(__has_cpp_attribute(attribute_token) >= test_value)
#else
	#define GUIORGY_ATTRIBUTE_AVAILABLE(attribute_token, test_value, cpp_version) \
		(__cplusplus >= cpp_version)
#endif

// A helper macro to drop the explanation argument of the [[nodiscard]] attribute on compilers that don't support it.
#if !GUIORGY_ATTRIBUTE_AVAILABLE(nodiscard, 201907L, 202002L)
	#ifdef nodiscard
		#define GUIORGY_nodiscard_BEFORE
		#undef nodiscard
	#endif
	#define nodiscard(explanation) nodiscard
#endif

// Helper macros to drop the [[likely]] and [[unlikely]] attributes on compilers that don't support them.
#ifdef LIKELY
	#define GUIORGY_LIKELY_BEFORE
	#undef LIKELY
#endif
#ifdef UNLIKELY
	#define GUIORGY_UNLIKELY_BEFORE
	#undef UNLIKELY
#endif
#if GUIORGY_ATTRIBUTE_AVAILABLE(likely, 201803L, 202002L) && GUIORGY_ATTRIBUTE_AVAILABLE(unlikely, 201803L, 202002L)
	#define LIKELY [[likely]]
	#define UNLIKELY [[unlikely]]
#else
	#define LIKELY
	#define UNLIKELY
#endif

// A helper macro to drop the constexpr specifier on destructors in unsupported C++ versions.
#ifdef CONSTEXPR_DESTRUCTOR
	#define GUIORGY_CONSTEXPR_DESTRUCTOR_BEFORE CONSTEXPR_DESTRUCTOR
#endif
#if __cplusplus >= 202002L
	#define CONSTEXPR_DESTRUCTOR constexpr
#else
	#define CONSTEXPR_DESTRUCTOR
#endif

// Forward declarations.
namespace guiorgy {
	namespace detail {
		// Forward declaration of is_flags_enum.
		template<typename>
		struct is_flags_enum;

		// Forward declaration of LruCacheOptions.
		enum struct LruCacheOptions : std::uint_fast8_t;

		// Forward declaration of lru_cache_opts.
		template<const LruCacheOptions, typename, typename, const std::size_t, template<typename...> class, typename, typename, typename...>
		class lru_cache_opts;

		// Forward declaration of vector_list.
		template<typename, const std::size_t>
		class vector_list;
	} // detail
} // guiorgy

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

	// Check if the specific type is nothrow move constructible, or nothrow copy constructible if it isn't move constructible.
	template<typename T>
	struct is_nothrow_move_or_copy_constructible final :
		std::conditional_t<
			std::is_move_constructible_v<T>
				? std::is_nothrow_move_constructible_v<T>
				: (std::is_copy_constructible_v<T> && std::is_nothrow_copy_constructible_v<T>),
			std::true_type,
			std::false_type
		> {};
	// Helper for is_nothrow_move_or_copy_constructible.
	template<typename T>
	inline constexpr bool is_nothrow_move_or_copy_constructible_v = is_nothrow_move_or_copy_constructible<T>::value;

	// Check if the specific type is nothrow move assignable, or nothrow copy assignable if it isn't move assignable.
	template<typename T>
	struct is_nothrow_move_or_copy_assignable final :
		std::conditional_t<
			std::is_move_assignable_v<T>
				? std::is_nothrow_move_assignable_v<T>
				: (std::is_copy_assignable_v<T> && std::is_nothrow_copy_assignable_v<T>),
			std::true_type,
			std::false_type
		> {};
	// Helper for is_nothrow_move_or_copy_assignable.
	template<typename T>
	inline constexpr bool is_nothrow_move_or_copy_assignable_v = is_nothrow_move_or_copy_assignable<T>::value;

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
} // guiorgy::detail

// Type qualifiert trait utils.
namespace guiorgy::detail {
	// The base type of the TypeQualifier flags enum.
	using TypeQualifierBaseType = std::uint_fast8_t;

	// Represents C++ type qualifiers. This is a flags enum.
	enum struct TypeQualifier : TypeQualifierBaseType {
		None                    = 0b0000'0000u,                    // T
		Const                   = 0b0000'0001u,                    // const T
		RightConst              = 0b0000'0010u,                    // T const (equivalent to const T)
		Pointer                 = 0b0000'0100u,                    // T*
		Reference               = 0b0000'1000u,                    // T&
		RValueReference         = 0b0001'0000u,                    // T&&

		// Flag combinations.
		ConstPointer            = Const | Pointer,                 // const T*
		ConstReference          = Const | Reference,               // const T&
		ConstRValueReference    = Const | RValueReference,         // const T&&
		PointerConst            = Pointer | RightConst,            // T* const
		ConstPointerConst       = Const | Pointer | RightConst,    // const T* const
		PointerReference        = Pointer | Reference,             // T*&
		ConstPointerReference   = Const | Pointer | Reference      // const T*&
	};

	// Checks whether the specified value is a valid TypeQualifier flag combination.
	inline constexpr bool is_valid_type_qualifier(const TypeQualifier type_qualifier) noexcept {
		return
			type_qualifier == TypeQualifier::None
			|| type_qualifier == TypeQualifier::Const
			|| type_qualifier == TypeQualifier::RightConst
			|| type_qualifier == TypeQualifier::Pointer
			|| type_qualifier == TypeQualifier::Reference
			|| type_qualifier == TypeQualifier::RValueReference
			|| type_qualifier == TypeQualifier::ConstPointer
			|| type_qualifier == TypeQualifier::ConstReference
			|| type_qualifier == TypeQualifier::ConstRValueReference
			|| type_qualifier == TypeQualifier::PointerConst
			|| type_qualifier == TypeQualifier::ConstPointerConst
			|| type_qualifier == TypeQualifier::PointerReference
			|| type_qualifier == TypeQualifier::ConstPointerReference;
	}

	// Define the flags enum operators for TypeQualifier.
	template<>
	struct is_flags_enum<TypeQualifier> final : std::true_type {};

	// Removes all qualifiers from a type.
	template<typename T>
	using remove_qualifiers = std::remove_cv_t<std::remove_reference_t<T>>;

	// Applies the specified qualifiers to the type.
	template<typename, const TypeQualifier type_qualifier>
	struct apply_qualifier final {
		static_assert(is_valid_type_qualifier(type_qualifier), "Invalid TypeQualifier");
	};

	// Returns T.
	template<typename T>
	struct apply_qualifier<T, TypeQualifier::None> {
		using type = remove_qualifiers<T>;
	};

	// Returns const T.
	template<typename T>
	struct apply_qualifier<T, TypeQualifier::Const> {
		using type = const remove_qualifiers<T>;
	};
	template<typename T>
	struct apply_qualifier<T, TypeQualifier::RightConst> {
		using type = const remove_qualifiers<T>;
	};

	// Returns T*.
	template<typename T>
	struct apply_qualifier<T, TypeQualifier::Pointer> {
		using type = remove_qualifiers<T>*;
	};

	// Returns T&.
	template<typename T>
	struct apply_qualifier<T, TypeQualifier::Reference> {
		using type = remove_qualifiers<T>&;
	};

	// Returns T&&.
	template<typename T>
	struct apply_qualifier<T, TypeQualifier::RValueReference> {
		using type = remove_qualifiers<T>&&;
	};

	// Returns const T*.
	template<typename T>
	struct apply_qualifier<T, TypeQualifier::ConstPointer> {
		using type = const remove_qualifiers<T>*;
	};

	// Returns T* const.
	template<typename T>
	struct apply_qualifier<T, TypeQualifier::PointerConst> {
		using type = remove_qualifiers<T>* const;
	};

	// Returns const T* const.
	template<typename T>
	struct apply_qualifier<T, TypeQualifier::ConstPointerConst> {
		using type = const remove_qualifiers<T>* const;
	};

	// Returns const T&.
	template<typename T>
	struct apply_qualifier<T, TypeQualifier::ConstReference> {
		using type = const remove_qualifiers<T>&;
	};

	// Returns const T&&.
	template<typename T>
	struct apply_qualifier<T, TypeQualifier::ConstRValueReference> {
		using type = const remove_qualifiers<T>&&;
	};

	// Returns T*&.
	template<typename T>
	struct apply_qualifier<T, TypeQualifier::PointerReference> {
		using type = remove_qualifiers<T>*&;
	};

	// Returns const T*&.
	template<typename T>
	struct apply_qualifier<T, TypeQualifier::ConstPointerReference> {
		using type = const remove_qualifiers<T>*&;
	};

	// Helper for apply_qualifier.
	template<typename T, const TypeQualifier type_qualifier>
	using apply_qualifier_t = typename apply_qualifier<T, type_qualifier>::type;
} // guiorgy::detail

// Type trait utils for hashmap.
namespace guiorgy::detail::hashmap {
	// SFINAE to check if the specified type has an emplace_hint member function similar to std::unordered_map.
	// The template returned when matching fails.
	template<typename, const TypeQualifier key_qualifier = TypeQualifier::None, const TypeQualifier mapped_qualifier = TypeQualifier::None, typename = void>
	struct has_emplace_hint final : std::false_type {};
	// The template returned when matching succeeds.
	template<typename T, const TypeQualifier key_qualifier, const TypeQualifier mapped_qualifier>
	struct has_emplace_hint<
		T,
		key_qualifier,
		mapped_qualifier,
		std::void_t<
			typename T::const_iterator,
			typename T::key_type,
			typename T::mapped_type,
			decltype(
				std::declval<T&>().emplace_hint(
					std::declval<typename T::const_iterator>(),
					std::declval<apply_qualifier_t<typename T::key_type, key_qualifier>>(),
					std::declval<apply_qualifier_t<typename T::mapped_type, mapped_qualifier>>()
				)
			)
		>
	> final : std::true_type {};
	// Helper for has_emplace_hint.
	template<typename T, const TypeQualifier key_qualifier = TypeQualifier::None, const TypeQualifier mapped_qualifier = TypeQualifier::None>
	inline constexpr bool has_emplace_hint_v = has_emplace_hint<T, key_qualifier, mapped_qualifier>::value;

	// SFINAE to check if the specified type has an insert member function that takes an iterator hint similar to std::unordered_map.
	// The template returned when matching fails.
	template<typename, const TypeQualifier value_qualifier = TypeQualifier::None, typename = void>
	struct has_insert_with_hint final : std::false_type {};
	// The template returned when matching succeeds.
	template<typename T, const TypeQualifier value_qualifier>
	struct has_insert_with_hint<
		T,
		value_qualifier,
		std::void_t<
			typename T::const_iterator,
			typename T::value_type,
			typename std::enable_if_t<is_pair_v<typename T::value_type>>,
			decltype(
				std::declval<T&>().insert(
					std::declval<typename T::const_iterator>(),
					std::declval<apply_qualifier_t<typename T::value_type, value_qualifier>>()
				)
			)
		>
	> final : std::true_type {};
	// Helper for has_insert_with_hint.
	template<typename T, const TypeQualifier value_qualifier = TypeQualifier::None>
	inline constexpr bool has_insert_with_hint_v = has_insert_with_hint<T, value_qualifier>::value;

	// SFINAE to check if the specified type has an insert member function similar to std::unordered_map.
	// The template returned when matching fails.
	template<typename, const TypeQualifier value_qualifier = TypeQualifier::None, typename = void>
	struct has_insert final : std::false_type {};
	// The template returned when matching succeeds.
	template<typename T, const TypeQualifier value_qualifier>
	struct has_insert<
		T,
		value_qualifier,
		std::void_t<
			typename T::value_type,
			typename std::enable_if_t<is_pair_v<typename T::value_type>>,
			decltype(
				std::declval<T&>().insert(
					std::declval<apply_qualifier_t<typename T::value_type, value_qualifier>>()
				)
			)
		>
	> final : std::true_type {};
	// Helper for has_insert.
	template<typename T, const TypeQualifier value_qualifier = TypeQualifier::None>
	inline constexpr bool has_insert_v = has_insert<T, value_qualifier>::value;

	// SFINAE to check if the specified type has an index operator[] similar to std::unordered_map.
	// The template returned when matching fails.
	template<typename, const TypeQualifier key_qualifier = TypeQualifier::None, const TypeQualifier mapped_qualifier = TypeQualifier::None, typename = void>
	struct has_index_operator final : std::false_type {};
	// The template returned when matching succeeds.
	template<typename T, const TypeQualifier key_qualifier, const TypeQualifier mapped_qualifier>
	struct has_index_operator<
		T,
		key_qualifier,
		mapped_qualifier,
		std::void_t<
			typename T::key_type,
			typename T::mapped_type,
			typename std::enable_if_t<
				std::is_same_v<
					apply_qualifier_t<typename T::mapped_type, mapped_qualifier>,
					decltype(
						std::declval<T&>().operator[](
							std::declval<apply_qualifier_t<typename T::key_type, key_qualifier>>()
						)
					)
				>
			>
		>
	> final : std::true_type {};
	// Helper for has_index_operator.
	template<typename T, const TypeQualifier key_qualifier = TypeQualifier::None, const TypeQualifier mapped_qualifier = TypeQualifier::None>
	inline constexpr bool has_index_operator_v = has_index_operator<T, key_qualifier, mapped_qualifier>::value;
} // guiorgy::detail::hashmap

// Utils.
namespace guiorgy::detail {
	#if __cplusplus < 202002L	// C++20
		// Returns the address of obj (implicitly converted to void*).
		// Based on LWG issue 3870 resolution.
		template<typename T>
		inline constexpr void* voidify(T& obj) noexcept(noexcept(std::addressof(std::declval<T&>()))) {
			return std::addressof(obj);
		}

		// Backport of std::construct_at.
		// Based on cppreference.
		template<typename T, typename... Args>
		inline constexpr T* construct_at(T* location, Args&&... args)
			noexcept(
				noexcept(voidify(std::declval<T&>()))
				&& noexcept(::new (std::declval<void*>()) T(std::declval<Args>()...))
			) {
			if constexpr (std::is_array_v<T>) {
				static_assert(sizeof...(Args) == 0, "If std::is_array_v<T> is true and sizeof...(Args) is nonzero, the program is ill-formed");

				return ::new (voidify(*location)) T[1]();
			} else {
				return ::new (voidify(*location)) T(std::forward<Args>(args)...);
			}
		}
	#else
		template<typename T, typename... Args>
		inline constexpr T* construct_at(T* location, Args&&... args)
			noexcept(
				noexcept(std::construct_at(std::declval<T*>(), std::forward<Args>(std::declval<Args>())...))
			) {
			return static_cast<T*>(std::construct_at(location, std::forward<Args>(args)...));
		}
	#endif

	// Emplaces a new object in place of the old.
	// If replace is set to false, assumes that the destination is uninitialized.
	// Remarks:
	//   - Calling emplace with an initialized object as the destination and
	//     replace explicitly set to false results in undefined behaviour.
	template<typename T, const bool replace = !std::is_trivially_destructible_v<T>, typename... Args>
	inline T& emplace(T* destination, Args&&... args)
		noexcept(
			(!replace || std::is_nothrow_destructible_v<T>)
			&& noexcept(construct_at(std::declval<T*>(), std::forward<Args>(std::declval<Args>())...))
		) {
		assert(destination != nullptr);

		if constexpr (replace) {
			static_assert(std::is_destructible_v<T>, "T needs to be destructible to allow emplacement");

			std::destroy_at(destination);
		}

		return *construct_at(destination, std::forward<Args>(args)...);
	}

	// Helper for emplace.
	template<typename T, const bool replace = !std::is_trivially_destructible_v<T>, typename... Args>
	inline T& emplace(T& destination, Args&&... args)
		noexcept(
			noexcept(std::addressof(std::declval<T&>()))
			&& noexcept(emplace<T, replace, Args...>(std::declval<T*>(), std::forward<Args>(std::declval<Args>())...))
		) {
		return emplace<T, replace, Args...>(std::addressof(destination), std::forward<Args>(args)...);
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
} // guiorgy::detail

// Wrapper utils.
namespace guiorgy::detail {
	// A wrapper for small integer types to prevent implicit integer promotion during arithmetic operations.
	// For example:
	//   uint8_t i1 = 1, i2 = 2;
	//   auto i3 = i1 + i2;
	//   static_assert(std::is_same_v<decltype(i3), uint8_t>); // fails
	//   static_assert(std::is_same_v<decltype(i3), int>); // passes
	template<typename int_t, typename promoted_t = std::size_t>
	class unpromoting final {
		static_assert(std::is_integral_v<int_t>, "int_t must be an integral type");
		static_assert(std::is_unsigned_v<int_t>, "Currently only unsigned integers are supported");
		static_assert(std::numeric_limits<promoted_t>::max() <= std::numeric_limits<std::size_t>::max(), "Currently only integers with size up to std::size_t are supported");
		static_assert(std::numeric_limits<int_t>::max() <= std::numeric_limits<promoted_t>::max(), "The size of promoted_t must be larger than that of int_t");

	public:
		using wrapped_type = int_t;
		using promoted_type = promoted_t;

	private:
		int_t _value;

	public:
		constexpr unpromoting() noexcept : _value(0u) {}
		CONSTEXPR_DESTRUCTOR ~unpromoting() = default;
		constexpr unpromoting(const unpromoting&) = default;
		constexpr unpromoting(unpromoting&&) = default;
		constexpr unpromoting& operator=(unpromoting const&) = default;
		constexpr unpromoting& operator=(unpromoting &&) = default;

		// Implicit conversions from the wrapped type.
		constexpr unpromoting(int_t value) noexcept : _value(value) {}
		constexpr unpromoting& operator=(int_t value) noexcept {
			_value = value;
			return *this;
		}

		// Implicit conversions to the wrapped type.
		constexpr operator int_t() const noexcept {
			return _value;
		}

		// Explicit conversions from other integer types.
		template<typename other_int_t, typename = std::enable_if_t<std::is_integral_v<other_int_t>>>
		constexpr explicit unpromoting(other_int_t value) noexcept : _value(static_cast<int_t>(value)) {}

		// Explicit conversions to the promoted type.
		constexpr explicit operator promoted_t() const noexcept {
			return static_cast<promoted_t>(_value);
		}
		constexpr promoted_t promote() const noexcept {
			return static_cast<promoted_t>(_value);
		}

		// Increment and decrement operators.
		constexpr unpromoting& operator++() noexcept {
			++_value;
			return *this;
		}
		constexpr unpromoting operator++(int) noexcept {
			unpromoting before = *this;
			this->operator++();
			return *this;
		}
		constexpr unpromoting& operator--() noexcept {
			--_value;
			return *this;
		}
		constexpr unpromoting operator--(int) noexcept {
			unpromoting before = *this;
			this->operator--();
			return *this;
		}

		// Arithmetic operators.
		constexpr unpromoting operator+(unpromoting other) const noexcept {
			return unpromoting(_value + other._value);
		}
		constexpr unpromoting& operator+=(unpromoting other) noexcept {
			_value += other._value;
			return *this;
		}
		constexpr unpromoting operator-(unpromoting other) const noexcept {
			return unpromoting(_value - other._value);
		}
		constexpr unpromoting& operator-=(unpromoting other) noexcept {
			_value -= other._value;
			return *this;
		}
		constexpr unpromoting operator*(unpromoting other) const noexcept{
			return unpromoting(_value * other._value);
		}
		constexpr unpromoting& operator*=(unpromoting other) noexcept {
			_value *= other._value;
			return *this;
		}
		constexpr unpromoting operator/(unpromoting other) const noexcept {
			return unpromoting(_value / other._value);
		}
		constexpr unpromoting& operator/=(unpromoting other) noexcept {
			_value /= other._value;
			return *this;
		}
		constexpr unpromoting operator%(unpromoting other) const noexcept {
			return unpromoting(_value % other._value);
		}
		constexpr unpromoting& operator%=(unpromoting other) noexcept {
			_value %= other._value;
			return *this;
		}

		// Arithmetic operators with integer literals.
		template<typename literal_t>
		constexpr std::enable_if_t<std::is_integral_v<literal_t>, unpromoting> operator+(literal_t&& other) const noexcept {
			return unpromoting(_value + static_cast<int_t>(other));
		}
		template<typename literal_t>
		constexpr std::enable_if_t<std::is_integral_v<literal_t>, unpromoting&> operator+=(literal_t&& other) const noexcept {
			_value += static_cast<int_t>(other);
			return *this;
		}
		template<typename literal_t>
		constexpr std::enable_if_t<std::is_integral_v<literal_t>, unpromoting> operator-(literal_t&& other) const noexcept {
			return unpromoting(_value - static_cast<int_t>(other));
		}
		template<typename literal_t>
		constexpr std::enable_if_t<std::is_integral_v<literal_t>, unpromoting&> operator-=(literal_t&& other) const noexcept {
			_value -= static_cast<int_t>(other);
			return *this;
		}
		template<typename literal_t>
		constexpr std::enable_if_t<std::is_integral_v<literal_t>, unpromoting> operator*(literal_t&& other) const noexcept {
			return unpromoting(_value * static_cast<int_t>(other));
		}
		template<typename literal_t>
		constexpr std::enable_if_t<std::is_integral_v<literal_t>, unpromoting&> operator*=(literal_t&& other) const noexcept {
			_value *= static_cast<int_t>(other);
			return *this;
		}
		template<typename literal_t>
		constexpr std::enable_if_t<std::is_integral_v<literal_t>, unpromoting> operator/(literal_t&& other) const noexcept {
			return unpromoting(_value / static_cast<int_t>(other));
		}
		template<typename literal_t>
		constexpr std::enable_if_t<std::is_integral_v<literal_t>, unpromoting&> operator/=(literal_t&& other) const noexcept {
			_value /= static_cast<int_t>(other);
			return *this;
		}
		template<typename literal_t>
		constexpr std::enable_if_t<std::is_integral_v<literal_t>, unpromoting> operator%(literal_t&& other) const noexcept {
			return unpromoting(_value % static_cast<int_t>(other));
		}
		template<typename literal_t>
		constexpr std::enable_if_t<std::is_integral_v<literal_t>, unpromoting&> operator%=(literal_t&& other) const noexcept {
			_value %= static_cast<int_t>(other);
			return *this;
		}

		// Comparison operators.
		constexpr bool operator==(unpromoting other) const noexcept {
			return _value == other._value;
		}
		constexpr bool operator!=(unpromoting other) const noexcept {
			return _value != other._value;
		}
		constexpr bool operator<(unpromoting other) const noexcept {
			return _value < other._value;
		}
		constexpr bool operator<=(unpromoting other) const noexcept {
			return _value <= other._value;
		}
		constexpr bool operator>(unpromoting other) const noexcept {
			return _value > other._value;
		}
		constexpr bool operator>=(unpromoting other) const noexcept {
			return _value >= other._value;
		}

		// Comparison operators with integer literals.
		template<typename literal_t>
		constexpr std::enable_if_t<std::is_integral_v<literal_t>, bool> operator==(literal_t&& other) const noexcept {
			return _value == static_cast<int_t>(other);
		}
		template<typename literal_t>
		constexpr std::enable_if_t<std::is_integral_v<literal_t>, bool> operator!=(literal_t&& other) const noexcept {
			return _value != static_cast<int_t>(other);
		}
		template<typename literal_t>
		constexpr std::enable_if_t<std::is_integral_v<literal_t>, bool> operator<(literal_t&& other) const noexcept {
			return _value < static_cast<int_t>(other);
		}
		template<typename literal_t>
		constexpr std::enable_if_t<std::is_integral_v<literal_t>, bool> operator<=(literal_t&& other) const noexcept {
			return _value <= static_cast<int_t>(other);
		}
		template<typename literal_t>
		constexpr std::enable_if_t<std::is_integral_v<literal_t>, bool> operator>(literal_t&& other) const noexcept {
			return _value > static_cast<int_t>(other);
		}
		template<typename literal_t>
		constexpr std::enable_if_t<std::is_integral_v<literal_t>, bool> operator>=(literal_t&& other) const noexcept {
			return _value >= static_cast<int_t>(other);
		}

		// Missing operators:
		//   - Bitwise Shift (<<, <<=, >>. >>=)
		//   - Bitwise (~, &, &=, |, |=, ^, ^=)
		//   - Boolean (!, bool())
		// Consider adding them if they are needed.
	};
} // guiorgy::detail

// Specializations of types inside std for our types.
namespace std {
	// std::numeric_limits specialization for uiorgy::detail::unpromoting.
	template<typename int_t, typename promoted_t>
	struct numeric_limits<guiorgy::detail::unpromoting<int_t, promoted_t>> {
		static constexpr bool is_specialized = true;
		static constexpr bool is_signed = std::numeric_limits<int_t>::is_signed;
		static constexpr bool is_integer = std::numeric_limits<int_t>::is_integer;
		static constexpr bool is_exact = std::numeric_limits<int_t>::is_exact;
		static constexpr bool has_infinity = std::numeric_limits<int_t>::has_infinity;
		static constexpr bool has_quiet_NaN = std::numeric_limits<int_t>::has_quiet_NaN;
		static constexpr bool has_signaling_NaN = std::numeric_limits<int_t>::has_signaling_NaN;
		[[deprecated("Deprecated in C++23")]] static constexpr std::float_denorm_style has_denorm = std::denorm_absent;
		static constexpr bool has_denorm_loss = std::numeric_limits<int_t>::has_denorm_loss;
		static constexpr std::float_round_style round_style = std::round_toward_zero;
		static constexpr bool is_iec559 = std::numeric_limits<int_t>::is_iec559;
		static constexpr bool is_bounded = std::numeric_limits<int_t>::is_bounded;
		static constexpr bool is_modulo = std::numeric_limits<int_t>::is_modulo;
		static constexpr int digits = std::numeric_limits<int_t>::digits;
		static constexpr int digits10 = std::numeric_limits<int_t>::digits10;
		static constexpr int max_digits10 = std::numeric_limits<int_t>::max_digits10;
		static constexpr int radix = std::numeric_limits<int_t>::radix;
		static constexpr int min_exponent = std::numeric_limits<int_t>::min_exponent;
		static constexpr int min_exponent10 = std::numeric_limits<int_t>::min_exponent10;
		static constexpr int max_exponent = std::numeric_limits<int_t>::max_exponent;
		static constexpr int max_exponent10 = std::numeric_limits<int_t>::max_exponent10;
		static constexpr bool traps = std::numeric_limits<int_t>::traps;
		static constexpr bool tinyness_before = std::numeric_limits<int_t>::tinyness_before;

		static constexpr guiorgy::detail::unpromoting<int_t, promoted_t> min() noexcept {
			return guiorgy::detail::unpromoting<int_t, promoted_t>(std::numeric_limits<int_t>::min());
		}
		static constexpr guiorgy::detail::unpromoting<int_t, promoted_t> lowest() noexcept {
			return guiorgy::detail::unpromoting<int_t, promoted_t>(std::numeric_limits<int_t>::lowest());
		}
		static constexpr guiorgy::detail::unpromoting<int_t, promoted_t> max() noexcept {
			return guiorgy::detail::unpromoting<int_t, promoted_t>(std::numeric_limits<int_t>::max());
		}

		static constexpr guiorgy::detail::unpromoting<int_t, promoted_t> epsilon() noexcept {
			return guiorgy::detail::unpromoting<int_t, promoted_t>(std::numeric_limits<int_t>::epsilon());
		}
		static constexpr guiorgy::detail::unpromoting<int_t, promoted_t> round_error() noexcept {
			return guiorgy::detail::unpromoting<int_t, promoted_t>(std::numeric_limits<int_t>::round_error());
		}

		static constexpr guiorgy::detail::unpromoting<int_t, promoted_t> infinity() noexcept {
			return guiorgy::detail::unpromoting<int_t, promoted_t>(std::numeric_limits<int_t>::infinity());
		}
		static constexpr guiorgy::detail::unpromoting<int_t, promoted_t> quiet_NaN() noexcept {
			return guiorgy::detail::unpromoting<int_t, promoted_t>(std::numeric_limits<int_t>::quiet_NaN());
		}
		static constexpr guiorgy::detail::unpromoting<int_t, promoted_t> signaling_NaN() noexcept {
			return guiorgy::detail::unpromoting<int_t, promoted_t>(std::numeric_limits<int_t>::signaling_NaN());
		}
		static constexpr guiorgy::detail::unpromoting<int_t, promoted_t> denorm_min() noexcept {
			return guiorgy::detail::unpromoting<int_t, promoted_t>(std::numeric_limits<int_t>::denorm_min());
		}
	};

	// std::is_integral specialization for uiorgy::detail::unpromoting.
	template<typename int_t, typename promoted_t>
	struct is_integral<guiorgy::detail::unpromoting<int_t, promoted_t>> : std::is_integral<int_t> {};

	// std::is_arithmetic specialization for uiorgy::detail::unpromoting.
	template<typename int_t, typename promoted_t>
	struct is_arithmetic<guiorgy::detail::unpromoting<int_t, promoted_t>> : std::is_arithmetic<int_t> {};

	// std::is_signed specialization for uiorgy::detail::unpromoting.
	template<typename int_t, typename promoted_t>
	struct is_signed<guiorgy::detail::unpromoting<int_t, promoted_t>> : std::is_signed<int_t> {};

	// std::is_unsigned specialization for uiorgy::detail::unpromoting.
	template<typename int_t, typename promoted_t>
	struct is_unsigned<guiorgy::detail::unpromoting<int_t, promoted_t>> : std::is_unsigned<int_t> {};
} // std

// Template operators for flags enums.
namespace guiorgy::detail {
	/*
	To enable the below operators for a specific flags enum, define the following template specialization for the said enum type:

	template<>
	struct is_flags_enum<EnumType> final : std::true_type {};
	*/

	// By default don't match any type.
	template<typename T>
	struct is_flags_enum final : std::false_type {};

	// Helper for is_flags_enum.
	template<typename T>
	inline constexpr bool is_flags_enum_v = is_flags_enum<T>::value;

	// Bitwise nagation operator for the the flags enum.
	template<typename EnumType>
	inline constexpr std::enable_if_t<is_flags_enum_v<EnumType>, EnumType> operator~(const EnumType x) noexcept {
		using EnumBaseType = typename std::underlying_type_t<EnumType>;
		const EnumBaseType _x = static_cast<EnumBaseType>(x);

		return static_cast<EnumType>(~_x);
	}

	// Bitwise or operator for the flags flags enum.
	template<typename EnumType>
	inline constexpr std::enable_if_t<is_flags_enum_v<EnumType>, EnumType> operator|(const EnumType a, const EnumType b) noexcept {
		using EnumBaseType = typename std::underlying_type_t<EnumType>;
		const EnumBaseType _a = static_cast<EnumBaseType>(a);
		const EnumBaseType _b = static_cast<EnumBaseType>(b);

		return static_cast<EnumType>(_a | _b);
	}
	template<typename EnumType>
	inline constexpr std::enable_if_t<is_flags_enum_v<EnumType>, EnumType>& operator|=(EnumType& a, const EnumType b) noexcept {
		a = a | b;
		return a;
	}

	// Bitwise and operator for the flags flags enum.
	template<typename EnumType>
	inline constexpr std::enable_if_t<is_flags_enum_v<EnumType>, bool> operator&(const EnumType a, const EnumType b) noexcept {
		using EnumBaseType = typename std::underlying_type_t<EnumType>;
		const EnumBaseType _a = static_cast<EnumBaseType>(a);
		const EnumBaseType _b = static_cast<EnumBaseType>(b);
		const EnumBaseType _none = static_cast<EnumBaseType>(0u);

		return (_a & _b) != _none;
	}

	// Bitwise XOR operator for the flags flags enum.
	template<typename EnumType>
	inline constexpr std::enable_if_t<is_flags_enum_v<EnumType>, EnumType> operator^(const EnumType a, const EnumType b) noexcept {
		using EnumBaseType = typename std::underlying_type_t<EnumType>;
		const EnumBaseType _a = static_cast<EnumBaseType>(a);
		const EnumBaseType _b = static_cast<EnumBaseType>(b);

		return static_cast<EnumType>(_a ^ _b);
	}
	template<typename EnumType>
	inline constexpr std::enable_if_t<is_flags_enum_v<EnumType>, EnumType>& operator^=(EnumType& a, const EnumType b) noexcept {
		a = a ^ b;
		return a;
	}

	// Boolean nagation operator for the flags flags enum.
	template<typename EnumType>
	inline constexpr bool operator!(const EnumType x) noexcept {
		using EnumBaseType = typename std::underlying_type_t<EnumType>;
		const EnumBaseType _x = static_cast<EnumBaseType>(x);
		const EnumBaseType _none = static_cast<EnumBaseType>(0u);

		return _x == _none;
	}

	// Binary substraction operator for the flags flags enum.
	// This is a equivalent to (a & ~b), in ther words, disable the bits that are enabled in the second flag.
	template<typename EnumType>
	inline constexpr std::enable_if_t<is_flags_enum_v<EnumType>, EnumType> operator-(const EnumType a, const EnumType b) noexcept {
		using EnumBaseType = typename std::underlying_type_t<EnumType>;
		const EnumBaseType _a = static_cast<EnumBaseType>(a);
		const EnumBaseType _b = static_cast<EnumBaseType>(b);

		return static_cast<EnumType>(_a & (~_b));
	}
	template<typename EnumType>
	inline constexpr std::enable_if_t<is_flags_enum_v<EnumType>, EnumType>& operator-=(EnumType& a, const EnumType b) noexcept {
		a = a - b;
		return a;
	}
} // guiorgy::detail

// Containers.
namespace guiorgy::detail {
	// A container where you can store elements and then take them out in an unspecified order.
	// Remarks:
	//   - The size of the container must not exceed the maximum value representable by index_t plus one.
	//   - The size limitation is not enforced within the container, the user must ensure that this condition is not violated.
	//   - The removed elements are not deleted immediately, instead they are replaced when new elements are put into the container.
	//   - Index operator[] of the internal std::vector are assumed to be noexcept, in other words, no out of bounds access is performed.
	template<typename T, typename _index_t = std::size_t>
	class vector_set final {
		static_assert(std::is_integral_v<_index_t> && std::is_unsigned_v<_index_t>, "_index_t must be an unsigned integer type");

		using index_t = unpromoting<_index_t, std::size_t>;

		std::vector<T> set{};
		index_t head = 0u;
		index_t tail = 0u;
		bool _empty = true;

		// Returns the index of the next element.
		template<const bool forward = true>
		[[nodiscard]] index_t next_index(const index_t index) const noexcept {
			assert(set.size() <= static_cast<std::size_t>(std::numeric_limits<index_t>::max()) + 1u);
			assert(set.size() != 0u);
			assert(std::size_t(index) < set.size());

			if constexpr (forward) {
				return index != index_t(safe_decrement(set.size())) ? index + 1u : index_t(0u);
			} else {
				return index != 0u ? index - 1u : index_t(safe_decrement(set.size()));
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
				std::size_t _head = std::size_t(head);
				std::size_t _tail = std::size_t(tail);

				return _tail <= _head ? _head - tail + 1u : (_head + 1u) + (set.size() - tail);
			}
		}

		// Returns the number of elements that the set has currently allocated space for.
		[[nodiscard]] std::size_t capacity() const noexcept {
			return set.capacity();
		}

		// Puts the given element into the set.
		void put(const T& value)
			noexcept(
				noexcept(std::declval<std::vector<T>>().push_back(std::declval<const T&>()))
				&& std::is_nothrow_copy_assignable_v<T>
			) {
			if (_empty) LIKELY {
				assert(head == tail);

				if (set.empty()) UNLIKELY {
					assert(head == 0u);

					set.push_back(value);
				} else {
					assert(head < set.size());

					set[head] = value;
				}

				_empty = false;
			} else {
				index_t next_head = next_index(head);

				if (next_head == tail) UNLIKELY {
					assert(size() == set.size());

					set.push_back(value);

					if (tail == 0u) LIKELY {
						++head;
					}
				} else {
					set[next_head] = value;
					head = next_head;
				}
			}

			assert(set.size() == 0u || set.size() - 1u <= std::numeric_limits<index_t>::max());
		}

		// Puts the given element into the set.
		void put(T&& value)
			noexcept(
				noexcept(std::declval<std::vector<T>>().push_back(std::declval<T&&>()))
				&& is_nothrow_move_or_copy_assignable_v<T>
			) {
			if (_empty) LIKELY {
				assert(head == tail);

				if (set.empty()) UNLIKELY {
					assert(head == 0u);

					set.push_back(std::move(value));
				} else {
					assert(head < set.size());

					set[head] = std::move(value);
				}

				_empty = false;
			} else {
				index_t next_head = next_index(head);

				if (next_head == tail) UNLIKELY {
					assert(size() == set.size());

					set.push_back(std::move(value));

					if (tail == 0u) LIKELY {
						++head;
					}
				} else {
					set[next_head] = std::move(value);
					head = next_head;
				}
			}

			assert(set.size() == 0u || set.size() - 1u <= std::numeric_limits<index_t>::max());
		}

		// Emplaces the given element into the set.
		template<typename... ValueArgs>
		void emplace(ValueArgs&&... value_args)
			noexcept(
				noexcept(std::declval<std::vector<T>>().emplace_back(std::declval<ValueArgs>()...))
				&& noexcept(detail::emplace(std::declval<T&>(), std::declval<ValueArgs>()...))
			) {
			if (_empty) LIKELY {
				assert(head == tail);

				if (set.empty()) UNLIKELY {
					assert(head == 0u);

					set.emplace_back(std::forward<ValueArgs>(value_args)...);
				} else {
					assert(head < set.size());

					detail::emplace(set[head], std::forward<ValueArgs>(value_args)...);
				}

				_empty = false;
			} else {
				index_t next_head = next_index(head);

				if (next_head == tail) UNLIKELY {
					assert(size() == set.size());

					set.emplace_back(std::forward<ValueArgs>(value_args)...);

					if (tail == 0u) LIKELY {
						++head;
					}
				} else {
					detail::emplace(set[next_head], std::forward<ValueArgs>(value_args)...);
					head = next_head;
				}
			}

			assert(set.size() == 0u || set.size() - 1u <= std::numeric_limits<index_t>::max());
		}

		// Returns a reference to the next element in the set.
		// Remarks:
		//   - Calling peek and accessing the returned element when the set was empty
		//     results in undefined behaviour.
		template<const bool from_head = true>
		[[nodiscard]] T& peek() const noexcept {
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
		// Remarks:
		//   - Calling take_ref and accessing the returned element when the set was empty
		//     results in undefined behaviour.
		template<const bool from_head = true>
		[[nodiscard]] T& take_ref() noexcept {
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
		// Remarks:
		//   - Calling take and accessing the returned element when the set was empty
		//     results in undefined behaviour.
		template<const bool from_head = true>
		[[nodiscard]] T take() noexcept {
			return take_ref<from_head>();
		}

		// Erases all elements from the set. After this call, size() returns zero.
		// References referring to contained elements are not invalidated, since the elements are deleted lazily.
		void clear() noexcept {
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
	//   - Index operator[] of the internal std::vector are assumed to be noexcept, in other words, no out of bounds access is performed.
	template<typename T, const std::size_t max_size = std::numeric_limits<std::size_t>::max() - 1u>
	class vector_list final {
		using index_t = uint_fit_t<max_size>;
		static constexpr const index_t null_index = std::numeric_limits<index_t>::max();
		static_assert(max_size <= null_index, "null_index can not be less than max_size, since those are valid indeces");

		// The internal node structure of the list.
		// Value first variant.
		struct list_node_value_first final {
			T value;
			index_t prior;
			index_t next;
#ifndef NDEBUG
			bool removed; // For debug assertions to check the correctness of the list.
#endif

			list_node_value_first(const index_t _prior, const index_t _next, const T& _value)
				noexcept(std::is_nothrow_copy_assignable_v<T>) :
				value(_value),
				prior(_prior),
				next(_next)
#ifndef NDEBUG
				, removed(false)
#endif
				{}

			list_node_value_first(const index_t _prior, const index_t _next, T&& _value)
				noexcept(is_nothrow_move_or_copy_assignable_v<T>) :
				value(std::move(_value)),
				prior(_prior),
				next(_next)
#ifndef NDEBUG
				, removed(false)
#endif
				{}

			template<typename... ValueArgs>
			list_node_value_first(const index_t _prior, const index_t _next, ValueArgs&&... value_args)
				noexcept(std::is_nothrow_constructible_v<T, ValueArgs...>) :
				value(std::forward<ValueArgs>(value_args)...),
				prior(_prior),
				next(_next)
#ifndef NDEBUG
				, removed(false)
#endif
				{}

			template<typename... ValueArgs>
			T& emplace_value(ValueArgs&&... value_args)
				noexcept(noexcept(emplace(std::declval<T&>(), std::declval<ValueArgs>()...))) {
				return emplace(value, std::forward<ValueArgs>(value_args)...);
			}

			list_node_value_first() = delete;
			~list_node_value_first() = default;
			list_node_value_first(const list_node_value_first&) = default;
			list_node_value_first(list_node_value_first&&) = default;
			list_node_value_first& operator=(list_node_value_first const&) = default;
			list_node_value_first& operator=(list_node_value_first &&) = default;
		};

		// The internal node structure of the list.
		// Value last variant.
		struct list_node_value_last final {
			index_t prior;
			index_t next;
			T value;
#ifndef NDEBUG
			bool removed; // For debug assertions to check the correctness of the list.
#endif

			list_node_value_last(const index_t _prior, const index_t _next, const T& _value)
				noexcept(std::is_nothrow_copy_assignable_v<T>) :
				prior(_prior),
				next(_next),
				value(_value)
#ifndef NDEBUG
				, removed(false)
#endif
				{}

			list_node_value_last(const index_t _prior, const index_t _next, T&& _value)
				noexcept(is_nothrow_move_or_copy_assignable_v<T>) :
				prior(_prior),
				next(_next),
				value(std::move(_value))
#ifndef NDEBUG
				, removed(false)
#endif
				{}

			template<typename... ValueArgs>
			list_node_value_last(const index_t _prior, const index_t _next, ValueArgs&&... value_args)
				noexcept(std::is_nothrow_constructible_v<T, ValueArgs...>) :
				prior(_prior),
				next(_next),
				value(std::forward<ValueArgs>(value_args)...)
#ifndef NDEBUG
				, removed(false)
#endif
				{}

			template<typename... ValueArgs>
			T& emplace_value(ValueArgs&&... value_args)
				noexcept(noexcept(emplace(std::declval<T&>(), std::declval<ValueArgs>()...))) {
				return emplace(value, std::forward<ValueArgs>(value_args)...);
			}

			list_node_value_last() = delete;
			~list_node_value_last() = default;
			list_node_value_last(const list_node_value_last&) = default;
			list_node_value_last(list_node_value_last&&) = default;
			list_node_value_last& operator=(list_node_value_last const&) = default;
			list_node_value_last& operator=(list_node_value_last &&) = default;
		};

		// The internal node structure of the list.
		using list_node = std::conditional_t<alignof(T) >= alignof(index_t), list_node_value_first, list_node_value_last>;

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
		[[nodiscard]] list_node& get_node(const index_t at) noexcept {
			assert(at < list.size());
#ifndef NDEBUG
			assert(!list[at].removed);
#endif

			return list[at];
		}

		// Returns a const reference to the node at the specified location.
		[[nodiscard]] const list_node& get_node(const index_t at) const noexcept {
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
		list_node& remove_node(const index_t at)
			noexcept(!mark_removed || noexcept(std::declval<vector_set<index_t, index_t>>().put(std::declval<index_t>()))) {
			assert(at < list.size());
#ifndef NDEBUG
			assert(!list[at].removed);
#endif

			list_node& _at = list[at];

			if constexpr (mark_removed) {
				free_indices.put(at);
#ifndef NDEBUG
				_at.removed = true;
#endif
			}

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

			return _at;
		}

		// Removes the node at the specified location.
		// References to the removed node are not invalidated, since the node is just marked as removed and added to the free_indices set.
		// Iterators to the removed node are invalidated. Other iterators are not affected.
		void erase_node(const index_t at)
			noexcept(noexcept(remove_node<true>(std::declval<index_t>()))) {
			[[maybe_unused]] const list_node& discard = remove_node<true>(at);
		}

		// Moves the node at the specified location to before/after another node and returns a reference to the moved node.
		// References to the moved/destination node are not invalidated, since only the prior and next members of the node are updated.
		// Iterators to the moved node are invalidated. Other iterators, including the iterators to the node at the movement destination, are not affected.
		// If before is true, then the node is moved before the node at the destination, otherwise, the node is moved after the destination.
		template<const bool before = true>
		list_node& move_node(const index_t from, const index_t to)
			noexcept(noexcept(remove_node<false>(std::declval<index_t>()))) {
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
		list_node& move_node_to_front(const index_t from)
			noexcept(noexcept(remove_node<false>(std::declval<index_t>()))) {
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
		list_node& move_node_to_back(const index_t from)
			noexcept(noexcept(remove_node<false>(std::declval<index_t>()))) {
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
		void reserve(const std::size_t capacity = max_size)
			noexcept(
				noexcept(std::declval<std::vector<list_node>>().reserve(std::declval<std::size_t>()))
				&& noexcept(std::declval<vector_set<index_t, index_t>>().reserve(std::declval<std::size_t>()))
			) {
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
		void push_front(const T& value) /*todo: noexcept*/ {
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

				list.emplace_back(head, null_index, value);

				if (head != null_index) LIKELY list[head].next = list_size;
				head = list_size;
				if (tail == null_index) UNLIKELY tail = head;
			}
		}

		// Prepends value to the beginning of the list.
		// If after the operation the new size() is greater than old capacity() a reallocation takes place, in which case all references are invalidated.
		// No iterators are invalidated.
		void push_front(T&& value) /*todo: noexcept*/ {
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

				list.emplace_back(head, null_index, std::move(value));

				if (head != null_index) LIKELY list[head].next = list_size;
				head = list_size;
				if (tail == null_index) UNLIKELY tail = head;
			}
		}

		// Prepends a new element to the beginning of the list.
		// The element is constructed in-place.
		// The arguments args... are forwarded to the constructor.
		// If after the operation the new size() is greater than old capacity() a reallocation takes place, in which case all references are invalidated.
		// No iterators are invalidated.
		template<typename... ValueArgs>
		T& emplace_front(ValueArgs&&... value_args) /*todo: noexcept*/ {
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

				T& value = list.emplace_back(head, null_index, std::forward<ValueArgs>(value_args)...).value;

				if (head != null_index) LIKELY list[head].next = list_size;
				head = list_size;
				if (tail == null_index) UNLIKELY tail = head;

				return value;
			}
		}

		// Appends a copy of value to the end of the list.
		// If after the operation the new size() is greater than old capacity() a reallocation takes place, in which case all references are invalidated.
		// No iterators are invalidated.
		void push_back(const T& value) /*todo: noexcept*/ {
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

				list.emplace_back(null_index, tail, value);

				if (tail != null_index) LIKELY list[tail].prior = list_size;
				tail = list_size;
				if (head == null_index) UNLIKELY head = tail;
			}
		}

		// Appends value to the end of the list.
		// If after the operation the new size() is greater than old capacity() a reallocation takes place, in which case all references are invalidated.
		// No iterators are invalidated.
		void push_back(T&& value) /*todo: noexcept*/ {
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

				list.emplace_back(null_index, tail, std::move(value));

				if (tail != null_index) LIKELY list[tail].prior = list_size;
				tail = list_size;
				if (head == null_index) UNLIKELY head = tail;
			}
		}

		// Appends a new element to the end of the list.
		// The element is constructed in-place.
		// The arguments args... are forwarded to the constructor.
		// If after the operation the new size() is greater than old capacity() a reallocation takes place, in which case all references are invalidated.
		// No iterators are invalidated.
		template<typename... ValueArgs>
		T& emplace_back(ValueArgs&&... value_args) /*todo: noexcept*/ {
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

				T& value = list.emplace_back(null_index, tail, std::forward<ValueArgs>(value_args)...).value;

				if (tail != null_index) LIKELY list[tail].prior = list_size;
				tail = list_size;
				if (head == null_index) UNLIKELY head = tail;

				return value;
			}
		}

		// Returns a reference to the first element in the list.
		[[nodiscard]] const T& front() noexcept {
			assert(head != null_index);
			assert(size() != 0u);

			return list[head].value;
		}

		// Returns a reference to the first element in the list.
		[[nodiscard]] T& front() const noexcept {
			assert(head != null_index);
			assert(size() != 0u);

			return list[head].value;
		}

		// Removes the first element of the list and returns a reference to the removed element.
		// References to the removed element are not invalidated, since the element is deleted lazily.
		// Iterators to the removed element are invalidated. Other iterators are not affected.
		T& pop_front_ref()
			noexcept(noexcept(std::declval<vector_set<index_t, index_t>>().put(std::declval<index_t>()))) {
			assert(head != null_index);
			assert(size() != 0u);
#ifndef NDEBUG
			assert(!list[head].removed);
#endif

			T& _front = list[head].value;

			free_indices.put(head);
#ifndef NDEBUG
			list[head].removed = true;
#endif

			head = _front.prior;
			if (head == null_index) UNLIKELY tail = null_index;
			else list[head].next = null_index;
			return _front;
		}

		// Removes the first element of the list and returns a copy of ithe removed element.
		// References to the removed element are not invalidated, since the element is deleted lazily.
		// Iterators to the removed element are invalidated. Other iterators are not affected.
		[[nodiscard("Use pop_front_ref() instead to avoid a needless copy of T")]]
		T pop_front() noexcept(noexcept(pop_front_ref())) {
			return pop_front_ref();
		}

		// Returns a reference to the last element in the list.
		[[nodiscard]] T& back() noexcept {
			assert(tail != null_index);
			assert(size() != 0u);

			return list[tail].value;
		}

		// Returns a reference to the last element in the list.
		[[nodiscard]] const T& back() const noexcept {
			assert(tail != null_index);
			assert(size() != 0u);

			return list[tail].value;
		}

		// Removes the last element of the list and returns a reference to the removed element.
		// References to the removed element are not invalidated, since the element is deleted lazily.
		// Iterators to the removed element are invalidated. Other iterators are not affected.
		T& pop_back_ref()
			noexcept(noexcept(std::declval<vector_set<index_t, index_t>>().put(std::declval<index_t>()))) {
			assert(tail != null_index);
			assert(size() != 0u);
#ifndef NDEBUG
			assert(!list[tail].removed);
#endif

			T& _back = list[tail].value;

			free_indices.put(tail);
#ifndef NDEBUG
			list[tail].removed = true;
#endif

			tail = _back.next;
			if (tail == null_index) UNLIKELY head = null_index;
			else list[tail].next = null_index;
			return _back;
		}

		// Removes the last element of the list and returns a copy of ithe removed element.
		// References to the removed element are not invalidated, since the element is deleted lazily.
		// Iterators to the removed element are invalidated. Other iterators are not affected.
		[[nodiscard("Use pop_back_ref() instead to avoid a needless copy of T")]]
		T pop_back() noexcept(noexcept(pop_back_ref())) {
			return pop_back_ref();
		}

		// Moves the element at the specified location to the front of the list and returns a reference to the moved element.
		// References to the moved/front element are not invalidated.
		// Iterators to the moved element are invalidated. Other iterators, including the iterators to the front element, are not affected.
		template<const bool reverse>
		T& move_to_front(const _iterator<false, reverse> it)
			noexcept(noexcept(move_node_to_front(std::declval<index_t>()))) {
			return move_node_to_front(it.forward_index()).value;
		}

		// Moves the element at the specified location to the back of the list and returns a reference to the moved element.
		// References to the moved/back element are not invalidated.
		// Iterators to the moved element are invalidated. Other iterators, including the iterators to the back element, are not affected.
		template<const bool reverse>
		T& move_to_back(const _iterator<false, reverse> it)
			noexcept(noexcept(move_node_to_back(std::declval<index_t>()))) {
			return move_node_to_back(it.forward_index()).value;
		}

		// Removes the element at the specified location and returns a reference to the removed element.
		// References to the removed element are not invalidated, since the element is deleted lazily.
		// Iterators to the removed element are invalidated. Other iterators are not affected.
		template<const bool reverse>
		T& erase(const _iterator<false, reverse> it)
			noexcept(noexcept(remove_node<true>(std::declval<index_t>()))) {
			return remove_node<true>(it.forward_index()).value;
		}

		// Erases all elements from the list. After this call, size() returns zero.
		// References referring to contained elements are not invalidated, since the elements are deleted lazily.
		// Invalidates any iterators referring to contained elements.
		void clear()
			noexcept(noexcept(std::declval<vector_set<index_t, index_t>>()._clear_and_fill_range(std::declval<std::size_t>()))) {
			free_indices._clear_and_fill_range(list.size());

#ifndef NDEBUG
			for (index_t index = head; index != null_index; index = list[index].prior) {
				assert(!list[index].removed);
				list[index].removed = true;
			}
			for (const list_node& node : list) {
				assert(node.removed);
			}
#endif

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

			template<const bool from_reverse, const bool to_reverse>
			static index_t reverse_index(list_pointer_t _list, const index_t index) noexcept {
				static_assert(from_reverse != to_reverse);
				assert(_list != nullptr);

				if constexpr (!from_reverse) {
					return index != null_index ? (*_list)[index].prior : null_index;
				} else {
					return index == null_index ? _list->tail : (*_list)[index].next;
				}
			}

			_iterator(list_reference_t _list, const index_t _index) noexcept : list(&_list), current_index(_index) {}

		public:
			// Copy constructor from (reverse_)iterator to const_(reverse_)iterator.
			template<const bool other_constant>
			_iterator(const _iterator<other_constant, reverse>& it) noexcept : list(it.list), current_index(it.current_index) {}

			// Copy constructor from (const_)iterator to (const_)reverse_iterator and from (const_)reverse_iterator to (const_)iterator.
			template<const bool other_reverse>
			_iterator(const _iterator<constant, other_reverse>& it) noexcept : list(it.list), current_index(reverse_index<other_reverse, reverse>(it.list, it.current_index)) {}

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
					assert(list != nullptr);

					return current_index == null_index ? list->tail : (*list)[current_index].next;
				}
			}

		public:
			reference operator*() const noexcept(noexcept(forward_index())) {
				assert(list != nullptr);

				return (*list)[forward_index()].value;
			}

			pointer operator->() const noexcept(noexcept(forward_index())) {
				assert(list != nullptr);

				return &((*list)[forward_index()].value);
			}

			_iterator& operator++() noexcept {
				assert(list != nullptr);

				if constexpr (!reverse) {
					if (current_index != null_index) current_index = (*list)[current_index].prior;
				} else {
					if (current_index != list->head) current_index = current_index == null_index ? list->tail : (*list)[current_index].next;
				}

				return *this;
			}

			_iterator operator++(int) noexcept {
				_iterator temp = *this;
				this->operator++();
				return temp;
			}

			_iterator& operator--() noexcept {
				assert(list != nullptr);

				if constexpr (!reverse) {
					if (current_index != list->head) current_index = current_index == null_index ? list->tail : (*list)[current_index].next;
				} else {
					if (current_index != null_index) current_index = (*list)[current_index].prior;
				}

				return *this;
			}

			_iterator operator--(int) noexcept {
				_iterator temp = *this;
				this->operator--();
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
		[[nodiscard]] list_node& operator[](const index_t position)
			noexcept(noexcept(get_node(std::declval<index_t>()))) {
			return get_node(position);
		}

		// See get_node for details.
		// This should only be used by _iterator;
		[[nodiscard]] const list_node& operator[](const index_t position) const
			noexcept(noexcept(get_node(std::declval<index_t>()))) {
			return get_node(position);
		}

		// Declare lru_cache_opts as a friend to give access to the dangerous member functions below.
		template<const LruCacheOptions, typename, typename, const std::size_t, template<typename...> class, typename, typename, typename...>
		friend class lru_cache_opts;

		// The functions below expose and operate internal indeces instead of using public iterators.
		// They are only indtended to be used by lru_cache_opts to avoid the overhead of using iterators.
		// Use them with caution.

		// See move_node_to_front for details.
		T& _move_value_at_to_front(const index_t position)
			noexcept(noexcept(move_node_to_front(std::declval<index_t>()))) {
			return move_node_to_front(position).value;
		}

		// See move_node_to_back for details.
		T& _move_value_at_to_back(const index_t position)
			noexcept(noexcept(move_node_to_back(std::declval<index_t>()))) {
			return move_node_to_back(position).value;
		}

		// See get_node for details.
		[[nodiscard]] T& _get_value_at(const index_t position)
			noexcept(noexcept(get_node(std::declval<index_t>()))) {
			return get_node(position).value;
		}

		// See get_node for details.
		[[nodiscard]] const T& _get_value_at(const index_t position) const
			noexcept(noexcept(get_node(std::declval<index_t>()))) {
			return get_node(position).value;
		}

		// See remove_node for details.
		T& _erase_value_at(const index_t position)
			noexcept(noexcept(remove_node<true>(std::declval<index_t>()))) {
			return remove_node<true>(position).value;
		}

		// See move_node_to_front for details.
		T& _move_last_value_to_front()
			noexcept(noexcept(_move_value_at_to_front(std::declval<index_t>()))) {
			return _move_value_at_to_front(tail);
		}

		// See move_node_to_back for details.
		T& _move_first_value_to_back()
			noexcept(noexcept(_move_value_at_to_back(std::declval<index_t>()))) {
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
	template<typename key_type>
	class DefaultHashFunction final {};
	template<typename key_type>
	class DefaultKeyEqualityPredicate final {};

	// The base type of the LruCacheOptions flags enum.
	using LruCacheOptionsBaseType = std::uint_fast8_t;

	// lru_cache_opts initialization options. This is a flags enum.
	enum struct LruCacheOptions : LruCacheOptionsBaseType {
		// No options set. Default initialization.
		None			= 0b0000'0000u,

		// Preallocate the capacity of the cache to a value that's greater or equal to max_size.
		Preallocate		= 0b0000'0001u
	};

	// Checks whether the specified value is a valid LruCacheOptions flag combination.
	inline constexpr bool is_valid_lru_cache_options(const LruCacheOptions options) noexcept {
		return
			options == LruCacheOptions::None
			|| options == LruCacheOptions::Preallocate;
	}

	// Define the flags enum operators for LruCacheOptions.
	template<>
	struct is_flags_enum<LruCacheOptions> final : std::true_type {};

	// The base type of the Likelihood enum.
	using LikelihoodBaseType = std::uint_fast8_t;

	// A hint to the likelihood of a condition being true.
	// Remarks:
	//   - This has no effect in C++ < 20.
	enum struct Likelihood : LikelihoodBaseType {
		Unknown,	// No attribute used
		Likely,		// [[likely]] attribute used
		Unlikely	// [[unlikely]] attribute used
	};

	// Checks whether the given likelihood value is a valid Likelihood enum value.
	inline constexpr bool is_valid_likelihood(const Likelihood likelihood) noexcept {
		return likelihood == Likelihood::Unknown || likelihood == Likelihood::Likely || likelihood == Likelihood::Unlikely;
	}

	// Converts a bool to a Likelihood::Likely or Likelihood::Unlikely enum.
	// The Likelihood::Unknown value is impossible to get this way.
	inline constexpr Likelihood likelihood(const bool likely) noexcept {
		return likely ? Likelihood::Likely : Likelihood::Unlikely;
	}

	// A size bounded container that contains key-value pairs with unique keys. Search, insertion, and removal of elements have
	// average constant-time complexity. Two keys are considered equivalent if key_equality_predicate predicate returns true when
	// passed those keys. If two keys are equivalent, the hash_function hash function must return the same value for both keys.
	// lru_cache_opts can not be created and used in the evaluation of a constant expression (constexpr). When filled, the container
	// uses the Least Recently Used replacement policy to store subsequent elements.
	// Remarks:
	//   - The inserted values and their members don't have stable memory addresses as they may be copied/moved when the cache grows.
	//     If that is required, or values are too expensive to copy/move, either use std::unique_ptr<value_t> or something similar
	//     as the value type instead, or tell lru_cache_opts to preallocate the needed storage by using reserve().
	//   - The removed elements are not deleted immediately, instead they are replaced when new elements are put into the container.
	//   - TODO: List the conditions that hashmap_template must satisfy and implement static_asserts to enforce that.
	//   - TODO: Check for the existence of .reserve() and only use if available.
	template<
		const LruCacheOptions options,
		typename key_type,
		typename value_type,
		const std::size_t max_size,
		template<typename...> class hashmap_template = std::unordered_map,
		typename hash_function = DefaultHashFunction<key_type>,
		typename key_equality_predicate = DefaultKeyEqualityPredicate<key_type>,
		typename... other_args
	>
	class lru_cache_opts final {
		static_assert(is_valid_lru_cache_options(options), "Invalid LruCacheOptions");
		static_assert(max_size != 0u, "max_size can not be 0");

		using key_value_pair_t = std::pair<key_type, value_type>;
		using list_type = vector_list<key_value_pair_t, max_size>;
		using list_index_type = typename list_type::index_t;

		static_assert(std::is_same_v<key_equality_predicate, DefaultKeyEqualityPredicate<key_type>> || !std::is_same_v<hash_function, DefaultHashFunction<key_type>>, "hash_function can't be default if key_equality_predicate is not default");
		static_assert(!std::is_same_v<key_equality_predicate, DefaultKeyEqualityPredicate<key_type>> || sizeof...(other_args) == 0u, "other_args can't be defined if key_equality_predicate is default");
		using hashmap_type = typename std::conditional_t<
			std::is_same_v<hash_function, DefaultHashFunction<key_type>>,
			hashmap_template<key_type, list_index_type>,
			typename std::conditional_t<
				std::is_same_v<key_equality_predicate, DefaultKeyEqualityPredicate<key_type>>,
				hashmap_template<key_type, list_index_type, hash_function>,
				hashmap_template<key_type, list_index_type, hash_function, key_equality_predicate, other_args...>
			>
		>;

		using hashmap_iterator_type = typename hashmap_type::iterator;
		using hashmap_const_iterator_type = typename hashmap_type::const_iterator;

		static constexpr bool hashmap_has_emplace_hint =
			hashmap::has_emplace_hint_v<hashmap_type, TypeQualifier::ConstReference, TypeQualifier::None>
			|| hashmap::has_emplace_hint_v<hashmap_type, TypeQualifier::ConstReference, TypeQualifier::ConstReference>
			|| hashmap::has_emplace_hint_v<hashmap_type, TypeQualifier::ConstReference, TypeQualifier::RValueReference>;
		static constexpr bool hashmap_has_insert_with_hint =
			hashmap::has_insert_with_hint_v<hashmap_type, TypeQualifier::ConstReference>
			|| hashmap::has_insert_with_hint_v<hashmap_type, TypeQualifier::RValueReference>;
		static constexpr bool hashmap_has_insert =
			hashmap::has_insert_v<hashmap_type, TypeQualifier::ConstReference>
			|| hashmap::has_insert_v<hashmap_type, TypeQualifier::RValueReference>;

	public:
		using iterator = typename list_type::const_iterator;
		using const_iterator = typename list_type::const_iterator;
		using reverse_iterator = typename list_type::const_reverse_iterator;
		using const_reverse_iterator = typename list_type::const_reverse_iterator;

	private:
		list_type _cache_items_list{};
		hashmap_type _cache_items_map{};

	public:
		template<typename... hashmap_args>
		explicit lru_cache_opts(hashmap_args&&... args) : _cache_items_list{}, _cache_items_map(std::forward<hashmap_args>(args)...) {
			if constexpr (options != LruCacheOptions::None) {
				if constexpr (options & LruCacheOptions::Preallocate) {
					reserve();
				}
			}
		}

		template<typename... hashmap_args>
		explicit lru_cache_opts(const LruCacheOptions runtime_options, hashmap_args&&... args) : _cache_items_list{}, _cache_items_map(std::forward<hashmap_args>(args)...) {
			static_assert(!options, "Runtime options override can only be used if compile-time options aren't used (are set to LruCacheOptions::None)");

			if (!is_valid_lru_cache_options(options)) {
				throw std::invalid_argument("Invalid LruCacheOptions runtime_options");
			}

			if (runtime_options & LruCacheOptions::Preallocate) {
				reserve();
			}
		}

		~lru_cache_opts() = default;
		lru_cache_opts(const lru_cache_opts&) = default;
		lru_cache_opts(lru_cache_opts&&) = default;
		lru_cache_opts& operator=(lru_cache_opts const&) = default;
		lru_cache_opts& operator=(lru_cache_opts &&) = default;

		// Checks whether the key inside the map matches the key inside the list element that the map points to.
		// Remarks:
		//   - This assumes that the key exists in the map.
		//   - Only used for debug assertions.
		[[nodiscard]] inline bool list_key_match_map_key(const hashmap_const_iterator_type map_it) const {
			assert(map_it != this->_cache_items_map.cend());

			const hashmap_const_iterator_type list_it = this->_cache_items_map.find(this->_cache_items_list._get_value_at(map_it->second).first);
			return list_it == map_it;
		}

	public:
		// If the key already exists in the container, copies value to the mapped value inside the container.
		// If the key doesn't exist in the container, inserts a copy of value into the container.
		// If a reallocation takes place after the operation, or the container size is already at max_size,
		// all references and iterators are invalidated.
		// Use key_exists and cache_full to hint to the compiler for which case to optimize for.
		template<const Likelihood key_exists = Likelihood::Unknown, const Likelihood cache_full = Likelihood::Unknown>
		void put(const key_type& key, const value_type& value) {
			assert(this->_cache_items_map.size() == this->_cache_items_list.size());

			hashmap_iterator_type it = this->_cache_items_map.find(key);
			assert(it == this->_cache_items_map.end() || list_key_match_map_key(it));

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
			} else {
				static_assert(max_size == 0u, "Unhandled value of Likelihood");
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
			} else {
				static_assert(max_size == 0u, "Unhandled value of Likelihood");
			}

			assert(!this->_cache_items_list.empty());
			if constexpr (this->hashmap_has_emplace_hint) {
				this->_cache_items_map.emplace_hint(it, key, this->_cache_items_list._first_value_index());
			} else if constexpr (this->hashmap_has_insert_with_hint) {
				this->_cache_items_map.insert(it, std::make_pair(key, this->_cache_items_list._first_value_index()));
			} else if constexpr (this->hashmap_has_insert) {
				this->_cache_items_map.insert(std::make_pair(key, this->_cache_items_list._first_value_index()));
			} else {
				this->_cache_items_map[key] = this->_cache_items_list._first_value_index();
			}
		}

		// See the put above for details.
		// Use key_likely_exists and cache_likely_full to hint to the compiler for which case to optimize for.
		template<const bool key_likely_exists, const bool cache_likely_full>
		void put(const key_type& key, const value_type& value) {
			put<likelihood(key_likely_exists), likelihood(cache_likely_full)>(key, value);
		}
		template<const bool key_likely_exists>
		void put(const key_type& key, const value_type& value) {
			put<likelihood(key_likely_exists)>(key, value);
		}

		// If the key already exists in the container, moves value to the mapped value inside the container.
		// If the key doesn't exist in the container, inserts value into the container.
		// If a reallocation takes place after the operation, or the container size is already at max_size,
		// all references and iterators are invalidated.
		// Use key_exists and cache_full to hint to the compiler for which case to optimize for.
		template<const Likelihood key_exists = Likelihood::Unknown, const Likelihood cache_full = Likelihood::Unknown>
		void put(const key_type& key, value_type&& value) {
			assert(this->_cache_items_map.size() == this->_cache_items_list.size());

			hashmap_iterator_type it = this->_cache_items_map.find(key);
			assert(it == this->_cache_items_map.end() || list_key_match_map_key(it));

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
			} else {
				static_assert(max_size == 0u, "Unhandled value of Likelihood");
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
			} else {
				static_assert(max_size == 0u, "Unhandled value of Likelihood");
			}

			assert(!this->_cache_items_list.empty());
			if constexpr (this->hashmap_has_emplace_hint) {
				this->_cache_items_map.emplace_hint(it, key, this->_cache_items_list._first_value_index());
			} else if constexpr (this->hashmap_has_insert_with_hint) {
				this->_cache_items_map.insert(it, std::make_pair(key, this->_cache_items_list._first_value_index()));
			} else if constexpr (this->hashmap_has_insert) {
				this->_cache_items_map.insert(std::make_pair(key, this->_cache_items_list._first_value_index()));
			} else {
				this->_cache_items_map[key] = this->_cache_items_list._first_value_index();
			}
		}

		// See the put above for details.
		// Use key_likely_exists and cache_likely_full to hint to the compiler for which case to optimize for.
		template<const bool key_likely_exists, const bool cache_likely_full>
		void put(const key_type& key, value_type&& value) {
			put<likelihood(key_likely_exists), likelihood(cache_likely_full)>(key, std::move(value));
		}
		template<const bool key_likely_exists>
		void put(const key_type& key, value_type&& value) {
			put<likelihood(key_likely_exists)>(key, std::move(value));
		}

		// If the key already exists in the container, constructs a new element in-place with the given value_args into the mapped value inside the container.
		// If the key doesn't exist in the container, constructs a new element in-place with the given value_args into the container.
		// If a reallocation takes place after the operation, or the container size is already at max_size,
		// all references and iterators are invalidated.
		// Use key_exists and cache_full to hint to the compiler for which case to optimize for.
		template<const Likelihood key_exists = Likelihood::Unknown, const Likelihood cache_full = Likelihood::Unknown, typename... ValueArgs>
		const value_type& emplace(const key_type& key, ValueArgs&&... value_args) {
			assert(this->_cache_items_map.size() == this->_cache_items_list.size());

			hashmap_iterator_type it = this->_cache_items_map.find(key);
			assert(it == this->_cache_items_map.end() || list_key_match_map_key(it));

			static_assert(is_valid_likelihood(key_exists), "key_exists has an invalid enum value for Likelihood");
			if constexpr (key_exists == Likelihood::Unknown) {
				if (it != this->_cache_items_map.end()) {
					value_type& value = this->_cache_items_list._get_value_at(it->second).second;
					emplace(value, std::forward<ValueArgs>(value_args)...);
					this->_cache_items_list._move_value_at_to_front(it->second);

					return value;
				}
			} else if constexpr (key_exists == Likelihood::Likely) {
				if (it != this->_cache_items_map.end()) LIKELY {
					value_type& value = this->_cache_items_list._get_value_at(it->second).second;
					emplace(value, std::forward<ValueArgs>(value_args)...);
					this->_cache_items_list._move_value_at_to_front(it->second);

					return value;
				}
			} else if constexpr (key_exists == Likelihood::Unlikely) {
				if (it != this->_cache_items_map.end()) UNLIKELY {
					value_type& value = this->_cache_items_list._get_value_at(it->second).second;
					emplace(value, std::forward<ValueArgs>(value_args)...);
					this->_cache_items_list._move_value_at_to_front(it->second);

					return value;
				}
			} else {
				static_assert(max_size == 0u, "Unhandled value of Likelihood");
			}

			value_type* value = nullptr;

			static_assert(is_valid_likelihood(cache_full), "cache_full has an invalid enum value for Likelihood");
			if constexpr (cache_full == Likelihood::Unknown) {
				if (this->_cache_items_map.size() == max_size) {
					key_value_pair_t& last = this->_cache_items_list.back();
					this->_cache_items_map.erase(last.first);
					last.first = key;
					emplace(last.second, std::forward<ValueArgs>(value_args)...);
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
					emplace(last.second, std::forward<ValueArgs>(value_args)...);
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
					emplace(last.second, std::forward<ValueArgs>(value_args)...);
					this->_cache_items_list._move_last_value_to_front();

					value = &last.second;
				} else {
					value = &(this->_cache_items_list.emplace_front(key, std::forward<ValueArgs>(value_args)...));
				}
			} else {
				static_assert(max_size == 0u, "Unhandled value of Likelihood");
			}

			assert(!this->_cache_items_list.empty());
			if constexpr (this->hashmap_has_emplace_hint) {
				this->_cache_items_map.emplace_hint(it, key, this->_cache_items_list._first_value_index());
			} else if constexpr (this->hashmap_has_insert_with_hint) {
				this->_cache_items_map.insert(it, std::make_pair(key, this->_cache_items_list._first_value_index()));
			} else if constexpr (this->hashmap_has_insert) {
				this->_cache_items_map.insert(std::make_pair(key, this->_cache_items_list._first_value_index()));
			} else {
				this->_cache_items_map[key] = this->_cache_items_list._first_value_index();
			}

			return *value;
		}

		// See the emplace above for details.
		// Use key_likely_exists and cache_likely_full to hint to the compiler for which case to optimize for.
		template<const bool key_likely_exists, const bool cache_likely_full, typename... ValueArgs>
		const value_type& emplace(const key_type& key, ValueArgs&&... value_args) {
			return emplace<likelihood(key_likely_exists), likelihood(cache_likely_full)>(key, std::forward<ValueArgs>(value_args)...);
		}
		template<const bool key_likely_exists, typename... ValueArgs>
		const value_type& emplace(const key_type& key, ValueArgs&&... value_args) {
			return emplace<likelihood(key_likely_exists)>(key, std::forward<ValueArgs>(value_args)...);
		}

		// Returns a std::optional with the value that is mapped to the given key,
		// or an empty optional if such key does not already exist.
		// Use key_exists to hint to the compiler for which case to optimize for.
		template<const Likelihood key_exists = Likelihood::Unknown>
		[[nodiscard("Use touch(const key_type& key) instead if all you want is to push the key to the bottom of the removal order")]]
		const std::optional<value_type> get(const key_type& key) {
			assert(this->_cache_items_map.size() == this->_cache_items_list.size());

			hashmap_iterator_type it = this->_cache_items_map.find(key);
			assert(it == this->_cache_items_map.end() || list_key_match_map_key(it));

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
			} else {
				static_assert(max_size == 0u, "Unhandled value of Likelihood");
			}
		}

		// See the get above for details.
		// Use key_likely_exists to hint to the compiler for which case to optimize for.
		template<const bool key_likely_exists>
		[[nodiscard("Use touch(const key_type& key) instead if all you want is to push the key to the bottom of the removal order")]]
		const std::optional<value_type> get(const key_type& key) {
			return get<likelihood(key_likely_exists)>(key);
		}

		// Returns a std::optional with a reference to the value that is mapped to
		// the given key, or an empty optional if such key does not already exist.
		// Use key_exists to hint to the compiler for which case to optimize for.
		// Remarks:
		//   - No guarantees are given about the underlying object lifetime when
		//     modifying the cache (inserting/removing elements), so use with caution.
		template<const Likelihood key_exists = Likelihood::Unknown>
		[[nodiscard("Use touch(const key_type& key) instead if all you want is to push the key to the bottom of the removal order")]]
		const std::optional<std::reference_wrapper<const value_type>> get_ref(const key_type& key) {
			assert(this->_cache_items_map.size() == this->_cache_items_list.size());

			hashmap_iterator_type it = this->_cache_items_map.find(key);
			assert(it == this->_cache_items_map.end() || list_key_match_map_key(it));

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
			} else {
				static_assert(max_size == 0u, "Unhandled value of Likelihood");
			}
		}

		// See the get_ref above for details.
		// Use key_likely_exists to hint to the compiler for which case to optimize for.
		template<const bool key_likely_exists>
		[[nodiscard("Use touch(const key_type& key) instead if all you want is to push the key to the bottom of the removal order")]]
		const std::optional<std::reference_wrapper<const value_type>> get_ref(const key_type& key) {
			return get_ref<likelihood(key_likely_exists)>(key);
		}

		// Returns true and copies the value that is mapped to the given key into
		// the given value_out reference, or false if such key does not already exist.
		// Use key_exists to hint to the compiler for which case to optimize for.
		// Remarks:
		//   - value_out is not modified when try_get returns false. If value_out was
		//     a reference to an uninitialized object, it will remain uninitialized,
		//     and accessing it will result in undefined behaviour.
		template<const Likelihood key_exists = Likelihood::Unknown>
		[[nodiscard("Use touch(const key_type& key) instead if all you want is to push the key to the bottom of the removal order")]]
		bool try_get(const key_type& key, value_type& value_out) {
			assert(this->_cache_items_map.size() == this->_cache_items_list.size());

			hashmap_iterator_type it = this->_cache_items_map.find(key);
			assert(it == this->_cache_items_map.end() || list_key_match_map_key(it));

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
			} else {
				static_assert(max_size == 0u, "Unhandled value of Likelihood");
			}
		}

		// See the try_get above for details.
		// Use key_likely_exists to hint to the compiler for which case to optimize for.
		template<const bool key_likely_exists>
		[[nodiscard("Use touch(const key_type& key) instead if all you want is to push the key to the bottom of the removal order")]]
		bool try_get(const key_type& key, value_type& value_out) {
			return try_get<likelihood(key_likely_exists)>(key, value_out);
		}

		// Returns true and assigns the address to the value that is mapped to the
		// given key into the given value_out pointer reference, or false if such
		// key does not already exist.
		// Use key_exists to hint to the compiler for which case to optimize for.
		// Remarks:
		//   - No guarantees are given about the underlying object lifetime when
		//     modifying the cache (inserting/removing elements), so use with caution.
		//   - value_out is not modified when try_get_ref returns false. If value_out
		//     was pointing to an invalid memory (e.g. nullptr), it will remain
		//     invalid, and dereferencing it will result in undefined behaviour.
		template<const Likelihood key_exists = Likelihood::Unknown>
		[[nodiscard("Use touch(const key_type& key) instead if all you want is to push the key to the bottom of the removal order")]]
		bool try_get_ref(const key_type& key, const value_type*& value_out) {
			assert(this->_cache_items_map.size() == this->_cache_items_list.size());

			hashmap_iterator_type it = this->_cache_items_map.find(key);
			assert(it == this->_cache_items_map.end() || list_key_match_map_key(it));

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
			} else {
				static_assert(max_size == 0u, "Unhandled value of Likelihood");
			}
		}

		// See the try_get_ref above for details.
		// Use key_likely_exists to hint to the compiler for which case to optimize for.
		template<const bool key_likely_exists>
		[[nodiscard("Use touch(const key_type& key) instead if all you want is to push the key to the bottom of the removal order")]]
		bool try_get_ref(const key_type& key, const value_type*& value_out) {
			return try_get_ref<likelihood(key_likely_exists)>(key, value_out);
		}

		// If the key exists in the container, removes the value that is mapped to the
		// given key and returns a std::optional with the removed value,
		// otherwise, returns an empty optional.
		// Use key_exists to hint to the compiler for which case to optimize for.
		template<const Likelihood key_exists = Likelihood::Unknown>
		[[nodiscard("Use erase(const key_type& key) instead")]]
		std::optional<value_type> remove(const key_type& key) {
			assert(this->_cache_items_map.size() == this->_cache_items_list.size());

			hashmap_iterator_type it = this->_cache_items_map.find(key);
			assert(it == this->_cache_items_map.end() || list_key_match_map_key(it));

			static_assert(is_valid_likelihood(key_exists), "key_exists has an invalid enum value for Likelihood");
			if constexpr (key_exists == Likelihood::Unknown) {
				if (it != this->_cache_items_map.end()) {
					value_type& value = this->_cache_items_list._erase_value_at(it->second).second;
					this->_cache_items_map.erase(it);
					return value;
				} else {
					return std::nullopt;
				}
			} else if constexpr (key_exists == Likelihood::Likely) {
				if (it != this->_cache_items_map.end()) LIKELY {
					value_type& value = this->_cache_items_list._erase_value_at(it->second).second;
					this->_cache_items_map.erase(it);
					return value;
				} else {
					return std::nullopt;
				}
			} else if constexpr (key_exists == Likelihood::Unlikely) {
				if (it != this->_cache_items_map.end()) UNLIKELY {
					value_type& value = this->_cache_items_list._erase_value_at(it->second).second;
					this->_cache_items_map.erase(it);
					return value;
				} else {
					return std::nullopt;
				}
			} else {
				static_assert(max_size == 0u, "Unhandled value of Likelihood");
			}
		}

		// See the remove above for details.
		// Use key_likely_exists to hint to the compiler for which case to optimize for.
		template<const bool key_likely_exists>
		[[nodiscard("Use erase(const key_type& key) instead")]]
		std::optional<value_type> remove(const key_type& key) {
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
		[[nodiscard("Use erase(const key_type& key) instead")]]
		std::optional<std::reference_wrapper<value_type>> remove_ref(const key_type& key) {
			assert(this->_cache_items_map.size() == this->_cache_items_list.size());

			hashmap_iterator_type it = this->_cache_items_map.find(key);
			assert(it == this->_cache_items_map.end() || list_key_match_map_key(it));

			static_assert(is_valid_likelihood(key_exists), "key_exists has an invalid enum value for Likelihood");
			if constexpr (key_exists == Likelihood::Unknown) {
				if (it != this->_cache_items_map.end()) {
					value_type& value = this->_cache_items_list._erase_value_at(it->second).second;
					this->_cache_items_map.erase(it);
					return std::make_optional(std::ref(value));
				} else {
					return std::nullopt;
				}
			} else if constexpr (key_exists == Likelihood::Likely) {
				if (it != this->_cache_items_map.end()) LIKELY {
					value_type& value = this->_cache_items_list._erase_value_at(it->second).second;
					this->_cache_items_map.erase(it);
					return std::make_optional(std::ref(value));
				} else {
					return std::nullopt;
				}
			} else if constexpr (key_exists == Likelihood::Unlikely) {
				if (it != this->_cache_items_map.end()) UNLIKELY {
					value_type& value = this->_cache_items_list._erase_value_at(it->second).second;
					this->_cache_items_map.erase(it);
					return std::make_optional(std::ref(value));
				} else {
					return std::nullopt;
				}
			} else {
				static_assert(max_size == 0u, "Unhandled value of Likelihood");
			}
		}

		// See the remove_ref above for details.
		// Use key_likely_exists to hint to the compiler for which case to optimize for.
		template<const bool key_likely_exists>
		[[nodiscard("Use erase(const key_type& key) instead")]]
		std::optional<std::reference_wrapper<value_type>> remove_ref(const key_type& key) {
			return remove_ref<likelihood(key_likely_exists)>(key);
		}

		// If the key exists in the container, removes the value that is mapped to the
		// given key, returns true and copies the value that is mapped to the given key
		// into the given value_out reference, or false if such key does not exist.
		// Use key_exists to hint to the compiler for which case to optimize for.
		// Remarks:
		//   - value_out is not modified when try_remove returns false. If value_out
		//     was a reference to an uninitialized object, it will remain
		//     uninitialized, and accessing it will result in undefined behaviour.
		template<const Likelihood key_exists = Likelihood::Unknown>
		[[nodiscard("Use erase(const key_type& key) instead")]]
		bool try_remove(const key_type& key, value_type& value_out) {
			assert(this->_cache_items_map.size() == this->_cache_items_list.size());

			hashmap_iterator_type it = this->_cache_items_map.find(key);
			assert(it == this->_cache_items_map.end() || list_key_match_map_key(it));

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
			} else {
				static_assert(max_size == 0u, "Unhandled value of Likelihood");
			}
		}

		// See the try_remove above for details.
		// Use key_likely_exists to hint to the compiler for which case to optimize for.
		template<const bool key_likely_exists>
		[[nodiscard("Use erase(const key_type& key) instead")]]
		bool try_remove(const key_type& key, value_type& value_out) {
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
		//   - value_out is not modified when try_remove_ref returns false. If
		//     value_out was pointing to an invalid memory (e.g. nullptr), it will
		//     remain invalid, and dereferencing it will result in undefined behaviour.
		template<const Likelihood key_exists = Likelihood::Unknown>
		[[nodiscard("Use erase(const key_type& key) instead")]]
		bool try_remove_ref(const key_type& key, const value_type*& value_out) {
			assert(this->_cache_items_map.size() == this->_cache_items_list.size());

			hashmap_iterator_type it = this->_cache_items_map.find(key);
			assert(it == this->_cache_items_map.end() || list_key_match_map_key(it));

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
			} else {
				static_assert(max_size == 0u, "Unhandled value of Likelihood");
			}
		}

		// See the try_remove_ref above for details.
		// Use key_likely_exists to hint to the compiler for which case to optimize for.
		template<const bool key_likely_exists>
		[[nodiscard("Use erase(const key_type& key) instead")]]
		bool try_remove_ref(const key_type& key, const value_type*& value_out) {
			return try_remove_ref<likelihood(key_likely_exists)>(key, value_out);
		}

		// If the key exists in the container, removes the value that is mapped to the
		// given key and returns true, or false if such key does not exist.
		// Use key_exists to hint to the compiler for which case to optimize for.
		template<const Likelihood key_exists = Likelihood::Unknown>
		bool erase(const key_type& key) {
			assert(this->_cache_items_map.size() == this->_cache_items_list.size());

			hashmap_iterator_type it = this->_cache_items_map.find(key);
			assert(it == this->_cache_items_map.end() || list_key_match_map_key(it));

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
			} else {
				static_assert(max_size == 0u, "Unhandled value of Likelihood");
			}
		}

		// See the erase above for details.
		// Use key_likely_exists to hint to the compiler for which case to optimize for.
		template<const bool key_likely_exists>
		bool erase(const key_type& key) {
			return erase<likelihood(key_likely_exists)>(key);
		}

		// Checks if the container contains an element with the given key.
		// This does not change the order in which elements are to be rewritten in.
		[[nodiscard]] bool exists(const key_type& key) const {
			assert(this->_cache_items_map.size() == this->_cache_items_list.size());

			const hashmap_const_iterator_type it = this->_cache_items_map.find(key);
			assert(it == this->_cache_items_map.cend() || list_key_match_map_key(it));

			return it != this->_cache_items_map.cend();
		}

		// Returns true and moves the value that is mapped to the given key to the
		// bottom of the removal order, or false if such key does not already exist.
		// Use key_exists to hint to the compiler for which case to optimize for.
		template<const Likelihood key_exists = Likelihood::Unknown>
		bool touch(const key_type& key) {
			assert(this->_cache_items_map.size() == this->_cache_items_list.size());

			hashmap_iterator_type it = this->_cache_items_map.find(key);
			assert(it == this->_cache_items_map.end() || list_key_match_map_key(it));

			static_assert(is_valid_likelihood(key_exists), "key_exists has an invalid enum value for Likelihood");
			if constexpr (key_exists == Likelihood::Unknown) {
				if (it != this->_cache_items_map.end()) {
					this->_cache_items_list._move_value_at_to_front(it->second);
					return true;
				} else {
					return false;
				}
			} else if constexpr (key_exists == Likelihood::Likely) {
				if (it != this->_cache_items_map.end()) LIKELY {
					this->_cache_items_list._move_value_at_to_front(it->second);
					return true;
				} else {
					return false;
				}
			} else if constexpr (key_exists == Likelihood::Unlikely) {
				if (it != this->_cache_items_map.end()) UNLIKELY {
					this->_cache_items_list._move_value_at_to_front(it->second);
					return true;
				} else {
					return false;
				}
			} else {
				static_assert(max_size == 0u, "Unhandled value of Likelihood");
			}
		}

		// See the touch above for details.
		// Use key_likely_exists to hint to the compiler for which case to optimize for.
		template<const bool key_likely_exists>
		bool touch(const key_type& key) {
			return touch<likelihood(key_likely_exists)>(key);
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

	// A helper type alias for lru_cache_opts with LruCacheOptions::None as the options.
	template<
		typename key_type,
		typename value_type,
		const std::size_t max_size,
		template<typename...> class hashmap_template = std::unordered_map,
		typename hash_function = detail::DefaultHashFunction<key_type>,
		typename key_equality_predicate = detail::DefaultKeyEqualityPredicate<key_type>,
		typename... other_args
	>
	using lru_cache = lru_cache_opts<LruCacheOptions::None, key_type, value_type, max_size, hashmap_template, hash_function, key_equality_predicate, other_args...>;
} // guiorgy::detail

// lru_cache public.
namespace guiorgy {
	// See detail::LruCacheOptions for details.
	using LruCacheOptions = detail::LruCacheOptions;

	// See detail::Likelihood for details.
	using Likelihood = detail::Likelihood;

	// See detail::lru_cache_opts for details.
	template<
		const LruCacheOptions options,
		typename key_type,
		typename value_type,
		const std::size_t max_size,
		template<typename...> class hashmap_template = std::unordered_map,
		typename hash_function = detail::DefaultHashFunction<key_type>,
		typename key_equality_predicate = detail::DefaultKeyEqualityPredicate<key_type>,
		typename... other_args
	>
	using lru_cache_opts = detail::lru_cache_opts<options, key_type, value_type, max_size, hashmap_template, hash_function, key_equality_predicate, other_args...>;

	// A helper type alias for lru_cache_opts with default hash_function and key_equality_predicate.
	template<
		const LruCacheOptions options,
		typename key_type,
		typename value_type,
		const std::size_t max_size,
		template<typename...> class hashmap_template = std::unordered_map,
		typename... other_args
	>
	using lru_cache_defhke_opts = lru_cache_opts<options, key_type, value_type, max_size, hashmap_template, detail::DefaultHashFunction<key_type>, detail::DefaultKeyEqualityPredicate<key_type>, other_args...>;

	// See detail::lru_cache for details.
	template<
		typename key_type,
		typename value_type,
		const std::size_t max_size,
		template<typename...> class hashmap_template = std::unordered_map,
		typename hash_function = detail::DefaultHashFunction<key_type>,
		typename key_equality_predicate = detail::DefaultKeyEqualityPredicate<key_type>,
		typename... other_args
	>
	using lru_cache = detail::lru_cache<key_type, value_type, max_size, hashmap_template, hash_function, key_equality_predicate, other_args...>;

	// A helper type alias for lru_cache_defhke_opts with LruCacheOptions::None as the options.
	template<
		typename key_type,
		typename value_type,
		const std::size_t max_size,
		template<typename...> class hashmap_template = std::unordered_map,
		typename... other_args
	>
	using lru_cache_defhke = lru_cache_defhke_opts<LruCacheOptions::None, key_type, value_type, max_size, hashmap_template, other_args...>;
} // guiorgy

// Restore CONSTEXPR_DESTRUCTOR if they were already defined.
#ifdef GUIORGY_CONSTEXPR_DESTRUCTOR_BEFORE
	#undef CONSTEXPR_DESTRUCTOR
	#define CONSTEXPR_DESTRUCTOR GUIORGY_CONSTEXPR_DESTRUCTOR_BEFORE
	#undef GUIORGY_CONSTEXPR_DESTRUCTOR_BEFORE
#endif

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

// Cleanup of GUIORGY_ATTRIBUTE_AVAILABLE.
#undef GUIORGY_ATTRIBUTE_AVAILABLE
