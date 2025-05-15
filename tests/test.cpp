#include "lrucache.hpp"
#include "gtest/gtest.h"

TEST(CacheTest1, SimplePut) {
	guiorgy::lru_cache<int, int, 1> cache_lru;
	cache_lru.put(7, 777);

	EXPECT_TRUE(cache_lru.exists(7));
	EXPECT_EQ(777, cache_lru.get(7).value());
	EXPECT_EQ(1, cache_lru.size());
}

TEST(CacheTest1, MissingValue) {
	guiorgy::lru_cache<int, int, 1> cache_lru;
	auto cached = cache_lru.get(7);

	EXPECT_FALSE(cached.has_value());
}

TEST(CacheTest2, KeepsAllValuesWithinCapacity) {
	constexpr int record_count = 100;
	constexpr int test_capacity = 50;

	guiorgy::lru_cache<int, int, test_capacity> cache_lru;

	for (int i = 0; i < record_count; ++i) {
		cache_lru.put(i, i);
	}

	for (int i = 0; i < record_count - test_capacity; ++i) {
		EXPECT_FALSE(cache_lru.exists(i));
	}

	for (int i = record_count - test_capacity; i < record_count; ++i) {
		EXPECT_TRUE(cache_lru.exists(i));

		auto cached = cache_lru.get(i);
		EXPECT_TRUE(cached.has_value());
		EXPECT_EQ(i, cached.value());
	}

	size_t size = cache_lru.size();
	EXPECT_EQ(test_capacity, size);
}

TEST(CacheTest2, HandlesOverwrites) {
	constexpr int test_capacity = 50;

	guiorgy::lru_cache<int, int, test_capacity> cache_lru;

	for (int i = 0; i < test_capacity; ++i) {
		cache_lru.put(i, i);
	}

	for (int i = 0; i < test_capacity; ++i) {
		cache_lru.put(i, -i);
	}

	for (int i = 0; i < test_capacity; ++i) {
		EXPECT_TRUE(cache_lru.exists(i));

		auto cached = cache_lru.get(i);
		EXPECT_TRUE(cached.has_value());
		EXPECT_EQ(-i, cached.value());
	}

	size_t size = cache_lru.size();
	EXPECT_EQ(test_capacity, size);
}

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	int ret = RUN_ALL_TESTS();
	return ret;
}
