#include <gtest/gtest.h>
#include "guiorgy/lrucache.hpp"
#include "math_utils.hpp"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wglobal-constructors"
#endif

#define TEST_GROUP DetailVectorSetBasicTests

TEST(TEST_GROUP, VectorSet) {
	constexpr int record_count = 5;

	guiorgy::detail::vector_set<int, unsigned char> set;

	for (int i = 0; i < record_count; ++i) {
		set.put(i);
		EXPECT_FALSE(set.empty());
		EXPECT_EQ(i + 1, set.size());
	}
	EXPECT_FALSE(set.empty());
	EXPECT_EQ(record_count, set.size());

	int sum = 0;
	for (int i = 0; i < record_count - 1; ++i) {
		sum += set.take();
	}
	EXPECT_FALSE(set.empty());
	EXPECT_EQ(1, set.size());
	EXPECT_EQ(range_sum(1, record_count - 1), sum);

	sum += set.take();
	EXPECT_TRUE(set.empty());
	EXPECT_EQ(0, set.size());
	EXPECT_EQ(range_sum(0, record_count - 1), sum);

	for (int i = 0; i < record_count; ++i) {
		set.put(i);
		EXPECT_FALSE(set.empty());
		EXPECT_EQ(i + 1, set.size());
	}
	EXPECT_FALSE(set.empty());
	EXPECT_EQ(record_count, set.size());
}

#ifdef __clang__
#pragma clang diagnostic pop
#endif
