#ifndef SKIP_ALLOCATION_TESTS
#include "gtest/gtest.h"
#include "alloc_utils.hpp"
#include "lrucache.hpp"

#define TEST_GROUP LruCacheAllocationTests

#if LRU_CACHE_HASH_MAP_IMPLEMENTATION == ANKERL_UNORDERED_DENSE_MAP
	TEST(TEST_GROUP, ConstructorAllocates) {
		reset_allocation_count();
		guiorgy::lru_cache<int, int, 1> cache_lru;
		EXPECT_ALLOC(1, 32);
	}
#elif LRU_CACHE_HASH_MAP_IMPLEMENTATION == ANKERL_UNORDERED_DENSE_SEGMENTED_MAP
	TEST(TEST_GROUP, ConstructorAllocates) {
		reset_allocation_count();
		guiorgy::lru_cache<int, int, 1> cache_lru;
		EXPECT_ALLOC(2, 4104);
	}
#else
	TEST(TEST_GROUP, ConstructorDoesnNotAllocate) {
		reset_allocation_count();
		guiorgy::lru_cache<int, int, 1> cache_lru;
		EXPECT_ALLOC(0, 0);
	}
#endif // LRU_CACHE_HASH_MAP_IMPLEMENTATION
#endif // SKIP_ALLOCATION_TESTS
