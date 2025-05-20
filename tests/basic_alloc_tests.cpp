#ifndef SKIP_ALLOCATION_TESTS
#include "gtest/gtest.h"
#include "alloc_utils.hpp"

#define TEST_GROUP BasicAllocationTests

TEST(TEST_GROUP, NewAllocationAccounted) {
	reset_allocation_count();
	int* i_ptr = new int;
	*i_ptr = -1;
	EXPECT_EQ(-1, *i_ptr);
	EXPECT_ALLOC(1, sizeof(int));
	delete i_ptr;
}

TEST(TEST_GROUP, GlobalNewAllocationAccounted) {
	reset_allocation_count();
	int* i_ptr = ::new int;
	*i_ptr = -1;
	EXPECT_EQ(-1, *i_ptr);
	EXPECT_ALLOC(1, sizeof(int));
	delete i_ptr;
}

TEST(TEST_GROUP, NewArrayAllocationAccounted) {
	constexpr int size = 50;

	reset_allocation_count();
	int* i_ptr = new int[size];
	i_ptr[0] = -1;
	EXPECT_EQ(-1, i_ptr[0]);
	EXPECT_ALLOC(1, sizeof(int) * size);
	delete[] i_ptr;
}

TEST(TEST_GROUP, GlobalNewArrayAllocationAccounted) {
	constexpr int size = 50;

	reset_allocation_count();
	int* i_ptr = ::new int[size];
	i_ptr[0] = -1;
	EXPECT_EQ(-1, i_ptr[0]);
	EXPECT_ALLOC(1, sizeof(int) * size);
	delete[] i_ptr;
}
#endif // SKIP_ALLOCATION_TESTS
