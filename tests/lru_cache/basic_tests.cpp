#include "gtest/gtest.h"
#include "guiorgy/lrucache.hpp"
#include "hashmap.hpp"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wglobal-constructors"
#endif

#define TEST_GROUP LruCacheBasicTests

TEST(TEST_GROUP, SimplePut) {
	guiorgy::lru_cache<int, int, 1, HASH_MAP_TYPE> cache_lru;
	cache_lru.put(7, 777);

	EXPECT_TRUE(cache_lru.exists(7));
	EXPECT_EQ(777, cache_lru.get(7).value());
	EXPECT_EQ(1, cache_lru.size());
}

TEST(TEST_GROUP, MissingValue) {
	guiorgy::lru_cache<int, int, 1, HASH_MAP_TYPE> cache_lru;
	auto cached = cache_lru.get(7);

	EXPECT_FALSE(cached.has_value());
}

TEST(TEST_GROUP, MissingAfterManualRemoval) {
	guiorgy::lru_cache<int, int, 1, HASH_MAP_TYPE> cache_lru;
	cache_lru.put(7, 777);
	cache_lru.erase(7);
	auto cached = cache_lru.get(7);

	EXPECT_FALSE(cached.has_value());
	EXPECT_EQ(0, cache_lru.size());
}

TEST(TEST_GROUP, SizeIs0AfterClear) {
	guiorgy::lru_cache<int, int, 1, HASH_MAP_TYPE> cache_lru;
	cache_lru.put(7, 777);

	EXPECT_EQ(1, cache_lru.size());

	cache_lru.clear();
	EXPECT_EQ(0, cache_lru.size());
}

TEST(TEST_GROUP, PutWithHints) {
	guiorgy::lru_cache<int, int, 5, HASH_MAP_TYPE> cache_lru;

	cache_lru.put(1, 111);
	EXPECT_TRUE(cache_lru.exists(1));
	EXPECT_EQ(111, cache_lru.get(1).value());
	EXPECT_EQ(1, cache_lru.size());

	cache_lru.put<true>(2, 222);
	EXPECT_TRUE(cache_lru.exists(2));
	EXPECT_EQ(222, cache_lru.get(2).value());
	EXPECT_EQ(2, cache_lru.size());

	cache_lru.put<false, true>(3, 333);
	EXPECT_TRUE(cache_lru.exists(3));
	EXPECT_EQ(333, cache_lru.get(3).value());
	EXPECT_EQ(3, cache_lru.size());

	cache_lru.put<guiorgy::Likelihood::Likely>(4, 444);
	EXPECT_TRUE(cache_lru.exists(4));
	EXPECT_EQ(444, cache_lru.get(4).value());
	EXPECT_EQ(4, cache_lru.size());

	cache_lru.put<guiorgy::Likelihood::Unlikely, guiorgy::Likelihood::Likely>(5, 555);
	EXPECT_TRUE(cache_lru.exists(5));
	EXPECT_EQ(555, cache_lru.get(5).value());
	EXPECT_EQ(5, cache_lru.size());
}

TEST(TEST_GROUP, IteratorWithOneElement) {
	guiorgy::lru_cache<int, int, 1, HASH_MAP_TYPE> cache_lru;
	cache_lru.put(7, 777);

	EXPECT_EQ(7, cache_lru.begin()->key);
	EXPECT_EQ(777, cache_lru.begin()->value);
	EXPECT_EQ(7, cache_lru.cbegin()->key);
	EXPECT_EQ(777, cache_lru.cbegin()->value);
	EXPECT_EQ(7, cache_lru.rbegin()->key);
	EXPECT_EQ(777, cache_lru.rbegin()->value);
	EXPECT_EQ(7, cache_lru.crbegin()->key);
	EXPECT_EQ(777, cache_lru.crbegin()->value);
}

TEST(TEST_GROUP, IteratorConversion) {
	using lru_cache_t = guiorgy::lru_cache<int, int, 1, HASH_MAP_TYPE>;

	EXPECT_TRUE((std::is_same_v<lru_cache_t::iterator, lru_cache_t::const_iterator>));
	EXPECT_TRUE((std::is_same_v<lru_cache_t::reverse_iterator, lru_cache_t::const_reverse_iterator>));

	lru_cache_t cache_lru;
	cache_lru.put(7, 777);

	// From iterator.
	{
		lru_cache_t::iterator it = cache_lru.begin();
		EXPECT_EQ(7, it->key);
		EXPECT_EQ(777, it->value);

		// To const_iterator.
		lru_cache_t::const_iterator cit(it);
		EXPECT_EQ(7, cit->key);
		EXPECT_EQ(777, cit->value);

		// To reverse_iterator.
		lru_cache_t::reverse_iterator rit(it);
		EXPECT_EQ(7, rit->key);
		EXPECT_EQ(777, rit->value);

		// To const_reverse_iterator.
		lru_cache_t::const_reverse_iterator crit(it);
		EXPECT_EQ(7, crit->key);
		EXPECT_EQ(777, crit->value);
	}

	// From const_iterator.
	{
		lru_cache_t::const_iterator cit = cache_lru.cbegin();
		EXPECT_EQ(7, cit->key);
		EXPECT_EQ(777, cit->value);

		// To iterator.
		lru_cache_t::iterator it(cit);
		EXPECT_EQ(7, it->key);
		EXPECT_EQ(777, it->value);

		// To reverse_iterator.
		lru_cache_t::reverse_iterator rit(cit);
		EXPECT_EQ(7, rit->key);
		EXPECT_EQ(777, rit->value);

		// To const_reverse_iterator.
		lru_cache_t::const_reverse_iterator crit(cit);
		EXPECT_EQ(7, crit->key);
		EXPECT_EQ(777, crit->value);
	}

	// From reverse_iterator.
	{
		lru_cache_t::reverse_iterator rit = cache_lru.rbegin();
		EXPECT_EQ(7, rit->key);
		EXPECT_EQ(777, rit->value);

		// To iterator.
		lru_cache_t::iterator it(rit);
		EXPECT_EQ(7, it->key);
		EXPECT_EQ(777, it->value);

		// To const_iterator.
		lru_cache_t::const_iterator cit(rit);
		EXPECT_EQ(7, cit->key);
		EXPECT_EQ(777, cit->value);

		// To const_reverse_iterator.
		lru_cache_t::const_reverse_iterator crit(rit);
		EXPECT_EQ(7, crit->key);
		EXPECT_EQ(777, crit->value);
	}

	// From const_reverse_iterator.
	{
		lru_cache_t::const_reverse_iterator crit = cache_lru.crbegin();
		EXPECT_EQ(7, crit->key);
		EXPECT_EQ(777, crit->value);

		// To iterator.
		lru_cache_t::iterator it(crit);
		EXPECT_EQ(7, it->key);
		EXPECT_EQ(777, it->value);

		// To const_iterator.
		lru_cache_t::const_iterator cit(crit);
		EXPECT_EQ(7, cit->key);
		EXPECT_EQ(777, cit->value);

		// To reverse_iterator.
		lru_cache_t::reverse_iterator rit(crit);
		EXPECT_EQ(7, rit->key);
		EXPECT_EQ(777, rit->value);
	}
}

TEST(TEST_GROUP, KeepsAllValuesWithinCapacity) {
	constexpr int record_count = 100;
	constexpr int test_capacity = 50;

	guiorgy::lru_cache<int, int, test_capacity, HASH_MAP_TYPE> cache_lru;

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

	guiorgy::lru_cache<int, int, record_count, HASH_MAP_TYPE> cache_lru;

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

	guiorgy::lru_cache<int, int, record_count, HASH_MAP_TYPE> cache_lru;

	for (int i = 0; i < record_count; ++i) {
		cache_lru.put(i, i);
	}

	for (int i = 0; i < record_count / 2; ++i) {
		cache_lru.erase(i);
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

	guiorgy::lru_cache<int, int, record_count, HASH_MAP_TYPE> cache_lru;

	for (int i = 0; i < record_count; ++i) {
		cache_lru.put(i, i);
	}

	for (int i = 0; i < record_count / 2; ++i) {
		cache_lru.erase(i);
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

TEST(TEST_GROUP, HandlesTouch) {
	constexpr int record_count = 50;

	guiorgy::lru_cache<int, int, record_count, HASH_MAP_TYPE> cache_lru;

	for (int i = 0; i < record_count; ++i) {
		cache_lru.put(i, i);
	}

	EXPECT_TRUE(cache_lru.exists(0));
	cache_lru.put(record_count, record_count);
	EXPECT_FALSE(cache_lru.exists(0));

	EXPECT_TRUE(cache_lru.exists(1));
	EXPECT_TRUE(cache_lru.exists(2));
	cache_lru.touch(1);
	cache_lru.put(record_count + 1, record_count + 1);
	EXPECT_TRUE(cache_lru.exists(1));
	EXPECT_FALSE(cache_lru.exists(2));

	cache_lru.touch(1);
	for (int i = 0; i < record_count - 1; ++i) {
		cache_lru.put(-i, i);
	}
	EXPECT_TRUE(cache_lru.exists(1));
	cache_lru.put(-record_count, record_count);
	EXPECT_FALSE(cache_lru.exists(1));

	size_t size = cache_lru.size();
	EXPECT_EQ(record_count, size);
}

#ifdef __clang__
#pragma clang diagnostic pop
#endif
