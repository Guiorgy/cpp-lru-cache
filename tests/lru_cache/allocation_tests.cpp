#ifndef SKIP_ALLOCATION_TESTS
#include "gtest/gtest.h"
#include "alloc_utils.hpp"
#include "guiorgy/lrucache.hpp"
#include "hashmap.hpp"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wglobal-constructors"
#endif

#define TEST_GROUP LruCacheAllocationTests

#if LRU_CACHE_HASH_MAP_IMPLEMENTATION == ANKERL_UNORDERED_DENSE_MAP
	TEST(TEST_GROUP, ConstructorAllocates) {
		reset_allocation_count();
		guiorgy::lru_cache<int, int, 1, HASH_MAP_TYPE> cache_lru;
		EXPECT_ALLOC(1, 32);
	}
#elif LRU_CACHE_HASH_MAP_IMPLEMENTATION == ANKERL_UNORDERED_DENSE_SEGMENTED_MAP
	TEST(TEST_GROUP, ConstructorAllocates) {
		reset_allocation_count();
		guiorgy::lru_cache<int, int, 1, HASH_MAP_TYPE> cache_lru;
		EXPECT_ALLOC(2, 4104);
	}
#else
	TEST(TEST_GROUP, ConstructorDoesnNotAllocate) {
		reset_allocation_count();
		guiorgy::lru_cache<int, int, 1, HASH_MAP_TYPE> cache_lru;
		EXPECT_NO_ALLOC();
	}
#endif // LRU_CACHE_HASH_MAP_IMPLEMENTATION

TEST(TEST_GROUP, ConstructorWithPreallocateAllocates) {
	reset_allocation_count();
	guiorgy::lru_cache_opts<guiorgy::LruCacheOptions::Preallocate, int, int, 1, HASH_MAP_TYPE> cache_lru;
	EXPECT_SOME_ALLOC();
}

#ifdef __clang__
#pragma clang diagnostic pop
#endif
#endif // SKIP_ALLOCATION_TESTS
