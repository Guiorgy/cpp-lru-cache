#include "gtest/gtest.h"
#include "lrucache.hpp"

#define TEST_GROUP LruCacheBasicTests

TEST(TEST_GROUP, SimplePut) {
	guiorgy::lru_cache<int, int, 1> cache_lru;
	cache_lru.put(7, 777);

	EXPECT_TRUE(cache_lru.exists(7));
	EXPECT_EQ(777, cache_lru.get(7).value());
	EXPECT_EQ(1, cache_lru.size());
}

TEST(TEST_GROUP, MissingValue) {
	guiorgy::lru_cache<int, int, 1> cache_lru;
	auto cached = cache_lru.get(7);

	EXPECT_FALSE(cached.has_value());
}

TEST(TEST_GROUP, SizeIs0AfterClear) {
	guiorgy::lru_cache<int, int, 1> cache_lru;
	cache_lru.put(7, 777);

	EXPECT_EQ(1, cache_lru.size());

	cache_lru.clear();
	EXPECT_EQ(0, cache_lru.size());
}

TEST(TEST_GROUP, KeepsAllValuesWithinCapacity) {
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

TEST(TEST_GROUP, HandlesOverwrites) {
	constexpr int record_count = 50;

	guiorgy::lru_cache<int, int, record_count> cache_lru;

	for (int i = 0; i < record_count; ++i) {
		cache_lru.put(i, i);
	}

	for (int i = 0; i < record_count; ++i) {
		cache_lru.put(i, -i);
	}

	for (int i = 0; i < record_count; ++i) {
		EXPECT_TRUE(cache_lru.exists(i));

		auto cached = cache_lru.get(i);
		EXPECT_TRUE(cached.has_value());
		EXPECT_EQ(-i, cached.value());
	}

	size_t size = cache_lru.size();
	EXPECT_EQ(record_count, size);
}

TEST(TEST_GROUP, HandlesRemovals) {
	constexpr int record_count = 50;

	guiorgy::lru_cache<int, int, record_count> cache_lru;

	for (int i = 0; i < record_count; ++i) {
		cache_lru.put(i, i);
	}

	for (int i = 0; i < record_count / 2; ++i) {
		cache_lru.remove(i);
	}

	for (int i = 0; i < record_count / 2; ++i) {
		EXPECT_FALSE(cache_lru.exists(i));
	}

	for (int i = record_count / 2 + 1; i < record_count; ++i) {
		EXPECT_TRUE(cache_lru.exists(i));

		auto cached = cache_lru.get(i);
		EXPECT_TRUE(cached.has_value());
		EXPECT_EQ(i, cached.value());
	}

	size_t size = cache_lru.size();
	EXPECT_EQ(record_count / 2, size);
}

TEST(TEST_GROUP, HandlesPutsAfterRemoval) {
	constexpr int record_count = 50;

	guiorgy::lru_cache<int, int, record_count> cache_lru;

	for (int i = 0; i < record_count; ++i) {
		cache_lru.put(i, i);
	}

	for (int i = 0; i < record_count / 2; ++i) {
		cache_lru.remove(i);
	}

	for (int i = record_count; i < 2 * record_count; ++i) {
		cache_lru.put(i, i);
	}

	for (int i = record_count; i < 2 * record_count; ++i) {
		EXPECT_TRUE(cache_lru.exists(i));

		auto cached = cache_lru.get(i);
		EXPECT_TRUE(cached.has_value());
		EXPECT_EQ(i, cached.value());
	}

	size_t size = cache_lru.size();
	EXPECT_EQ(record_count, size);
}
