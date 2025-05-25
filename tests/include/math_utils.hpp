#pragma once

#include <type_traits>
#include <limits>

template<typename int_t>
inline constexpr int_t range_sum(const int_t from, int_t to, const bool inclusive = true) noexcept {
	static_assert(std::is_integral_v<int_t>);

	if (!inclusive) {
		if (to == std::numeric_limits<int_t>::lowest()) {
			return 0;
		}

		--to;
	}

	if (to < from) {
		return 0;
	}

	return (to - from + 1) * (from + to) / 2;
}
