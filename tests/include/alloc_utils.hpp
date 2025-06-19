#pragma once

#ifndef SKIP_ALLOCATION_TESTS
#include <atomic>

extern std::atomic<std::size_t> allocation_count;
extern std::atomic<std::size_t> allocated_bytes;

void extern reset_allocation_count() noexcept;

// The identity/expansion macro to force an extra scan. Necessary on MSVC.
#define IDENTITY(x) x

// Takes a macro name and its arguments, and forwards them.
#define FORWARD_ARGS(macro, ...) IDENTITY(macro(__VA_ARGS__))

#define EXPECT_NO_ALLOC() \
	do { EXPECT_EQ(0u, allocation_count.load()); EXPECT_EQ(0u, allocated_bytes.load()); } while (false)

#define EXPECT_SOME_ALLOC() \
	do { EXPECT_NE(0u, allocation_count.load()); EXPECT_NE(0u, allocated_bytes.load()); } while (false)

#define EXPECT_ALLOC_COUNT(expected_count) \
	do { EXPECT_EQ(expected_count, allocation_count.load()); } while (false)

#define EXPECT_ALLOC_COUNT_AND_BYTES(expected_count, expected_bytes) \
	do { EXPECT_EQ(expected_count, allocation_count.load()); EXPECT_EQ(expected_bytes, allocated_bytes.load()); } while (false)

#define GET_EXPECT_ALLOC_MACRO(_1, _2, macro, ...) macro

#define EXPECT_ALLOC(...) \
	FORWARD_ARGS(GET_EXPECT_ALLOC_MACRO(__VA_ARGS__, EXPECT_ALLOC_COUNT_AND_BYTES, EXPECT_ALLOC_COUNT), __VA_ARGS__)
#endif // SKIP_ALLOCATION_TESTS
