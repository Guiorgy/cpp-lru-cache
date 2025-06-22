#ifndef SKIP_ALLOCATION_TESTS
#include <gtest/gtest.h>
#include "alloc_utils.hpp"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wglobal-constructors"
#endif

#define TEST_GROUP AllocUtilsTests

TEST(TEST_GROUP, NewAllocationAccounted) {
	reset_allocation_count();
	volatile int* i_ptr = new int{-1}; // cppcheck-suppress [legacyUninitvar,uninitvar]
	EXPECT_EQ(-1, *i_ptr);
	EXPECT_ALLOC(1, sizeof(int));
	delete i_ptr;
}

TEST(TEST_GROUP, GlobalNewAllocationAccounted) {
	reset_allocation_count();
	volatile int* i_ptr = ::new int{-1}; // cppcheck-suppress [legacyUninitvar,uninitvar]
	EXPECT_EQ(-1, *i_ptr);
	EXPECT_ALLOC(1, sizeof(int));
	delete i_ptr;
}

TEST(TEST_GROUP, NewArrayAllocationAccounted) {
	constexpr int size = 50;

	reset_allocation_count();
	volatile int* i_ptr = new int[size]{-1}; // cppcheck-suppress [legacyUninitvar,uninitvar]
	EXPECT_EQ(-1, i_ptr[0]);
	EXPECT_ALLOC(1, sizeof(int) * size);
	delete[] i_ptr;
}

TEST(TEST_GROUP, GlobalNewArrayAllocationAccounted) {
	constexpr int size = 50;

	reset_allocation_count();
	volatile int* i_ptr = ::new int[size]{-1}; // cppcheck-suppress [legacyUninitvar,uninitvar]
	EXPECT_EQ(-1, i_ptr[0]);
	EXPECT_ALLOC(1, sizeof(int) * size);
	delete[] i_ptr;
}

#ifdef __clang__
#pragma clang diagnostic pop
#endif
#endif // SKIP_ALLOCATION_TESTS
