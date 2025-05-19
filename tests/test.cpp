#include "lrucache.hpp"
#include "gtest/gtest.h"
#include <iostream>

// preallocate = false (default)
static_assert(std::is_constructible_v<guiorgy::lru_cache<int, int, 1>> == true);
static_assert(std::is_trivially_constructible_v<guiorgy::lru_cache<int, int, 1>> == false);
#if LRU_CACHE_HASH_MAP_IMPLEMENTATION == STL_UNORDERED_MAP
	static_assert(std::is_nothrow_constructible_v<guiorgy::lru_cache<int, int, 1>> == true);
#elif (LRU_CACHE_HASH_MAP_IMPLEMENTATION == ABSEIL_FLAT_HASH_MAP) || (LRU_CACHE_HASH_MAP_IMPLEMENTATION == TESSIL_SPARSE_MAP) || (LRU_CACHE_HASH_MAP_IMPLEMENTATION == ANKERL_UNORDERED_DENSE_MAP) || (LRU_CACHE_HASH_MAP_IMPLEMENTATION == ANKERL_UNORDERED_DENSE_SEGMENTED_MAP)
	static_assert(std::is_nothrow_constructible_v<guiorgy::lru_cache<int, int, 1>> == false);
#else
	#error "Unhandled case of LRU_CACHE_HASH_MAP_IMPLEMENTATION"
#endif

static_assert(std::is_default_constructible_v<guiorgy::lru_cache<int, int, 1>> == true);
static_assert(std::is_trivially_default_constructible_v<guiorgy::lru_cache<int, int, 1>> == false);
#if LRU_CACHE_HASH_MAP_IMPLEMENTATION == STL_UNORDERED_MAP
	static_assert(std::is_nothrow_default_constructible_v<guiorgy::lru_cache<int, int, 1>> == true);
#elif (LRU_CACHE_HASH_MAP_IMPLEMENTATION == ABSEIL_FLAT_HASH_MAP) || (LRU_CACHE_HASH_MAP_IMPLEMENTATION == TESSIL_SPARSE_MAP) || (LRU_CACHE_HASH_MAP_IMPLEMENTATION == ANKERL_UNORDERED_DENSE_MAP) || (LRU_CACHE_HASH_MAP_IMPLEMENTATION == ANKERL_UNORDERED_DENSE_SEGMENTED_MAP)
	static_assert(std::is_nothrow_default_constructible_v<guiorgy::lru_cache<int, int, 1>> == false);
#else
	#error "Unhandled case of LRU_CACHE_HASH_MAP_IMPLEMENTATION"
#endif

static_assert(std::is_copy_constructible_v<guiorgy::lru_cache<int, int, 1>> == true);
static_assert(std::is_trivially_copy_constructible_v<guiorgy::lru_cache<int, int, 1>> == false);
static_assert(std::is_nothrow_copy_constructible_v<guiorgy::lru_cache<int, int, 1>> == false);

static_assert(std::is_move_constructible_v<guiorgy::lru_cache<int, int, 1>> == true);
static_assert(std::is_trivially_move_constructible_v<guiorgy::lru_cache<int, int, 1>> == false);
static_assert(std::is_nothrow_move_constructible_v<guiorgy::lru_cache<int, int, 1>> == true);

// preallocate = true
static_assert(std::is_constructible_v<guiorgy::lru_cache<int, int, 1, true>> == true);
static_assert(std::is_trivially_constructible_v<guiorgy::lru_cache<int, int, 1, true>> == false);
static_assert(std::is_nothrow_constructible_v<guiorgy::lru_cache<int, int, 1, true>> == false);

// static_assert(std::is_default_constructible_v<guiorgy::lru_cache<int, int, 1, true>> == false); // This fails?!
static_assert(std::is_trivially_default_constructible_v<guiorgy::lru_cache<int, int, 1, true>> == false);
static_assert(std::is_nothrow_default_constructible_v<guiorgy::lru_cache<int, int, 1, true>> == false);

static_assert(std::is_copy_constructible_v<guiorgy::lru_cache<int, int, 1, true>> == true);
static_assert(std::is_trivially_copy_constructible_v<guiorgy::lru_cache<int, int, 1, true>> == false);
static_assert(std::is_nothrow_copy_constructible_v<guiorgy::lru_cache<int, int, 1, true>> == false);

static_assert(std::is_move_constructible_v<guiorgy::lru_cache<int, int, 1, true>> == true);
static_assert(std::is_trivially_move_constructible_v<guiorgy::lru_cache<int, int, 1, true>> == false);
static_assert(std::is_nothrow_move_constructible_v<guiorgy::lru_cache<int, int, 1, true>> == true);

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

TEST(CacheTest1, ClearReturnsSize0) {
	constexpr int test_capacity = 50;

	guiorgy::lru_cache<int, int, test_capacity> cache_lru;

	for (int i = 0; i < test_capacity; ++i) {
		cache_lru.put(i, i);
	}

	size_t size = cache_lru.size();
	EXPECT_EQ(test_capacity, size);

	cache_lru.clear();
	size = cache_lru.size();
	EXPECT_EQ(0, size);
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

TEST(CacheTest3, HandlesRemovals) {
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

TEST(CacheTest3, HandlesPutsAfterRemoval) {
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

#ifndef SKIP_ALLOCATION_TESTS
#include <cstdlib>
#include <atomic>
#include <new>

static std::atomic<std::size_t> allocation_count = 0u;
static std::atomic<std::size_t> allocated_bytes = 0u;

static inline void reset_allocation_count() noexcept {
	allocation_count.store(0u);
	allocated_bytes.store(0u);
}

#define EXPECT_NO_ALLOC() \
	do { EXPECT_EQ(0u, allocation_count.load()); EXPECT_EQ(0u, allocated_bytes.load()); } while (false)

#define EXPECT_ALLOC_COUNT(expected_count) \
	do { EXPECT_EQ(expected_count, allocation_count.load()); } while (false)

#define EXPECT_ALLOC_COUNT_AND_BYTES(expected_count, expected_bytes) \
	do { EXPECT_EQ(expected_count, allocation_count.load()); EXPECT_EQ(expected_bytes, allocated_bytes.load()); } while (false)

#define GET_EXPECT_ALLOC_MACRO(_1, _2, NAME, ...) NAME

#define EXPECT_ALLOC(...) \
	GET_EXPECT_ALLOC_MACRO(__VA_ARGS__, EXPECT_ALLOC_COUNT_AND_BYTES, EXPECT_ALLOC_COUNT)(__VA_ARGS__)

void* operator new(std::size_t size) {
	++allocation_count;
	allocated_bytes += size;
	return std::malloc(size);
}

void operator delete(void* ptr) noexcept {
	std::free(ptr);
}

void operator delete(void* ptr, std::size_t) noexcept {
	std::free(ptr);
}

void* operator new[](std::size_t size) {
	++allocation_count;
	allocated_bytes += size;
	return std::malloc(size);
}

void operator delete[](void* ptr) noexcept {
	std::free(ptr);
}

void operator delete[](void* ptr, std::size_t) noexcept {
	std::free(ptr);
}

TEST(AllocationCountingTest, NewAllocationAccounted) {
	reset_allocation_count();
	int* i_ptr = new int;
	*i_ptr = -1;
	EXPECT_EQ(-1, *i_ptr);
	EXPECT_ALLOC(1, sizeof(int));
	delete i_ptr;
}

TEST(AllocationCountingTest, GlobalNewAllocationAccounted) {
	reset_allocation_count();
	int* i_ptr = ::new int;
	*i_ptr = -1;
	EXPECT_EQ(-1, *i_ptr);
	EXPECT_ALLOC(1, sizeof(int));
	delete i_ptr;
}

TEST(AllocationCountingTest, NewArrayAllocationAccounted) {
	constexpr int size = 50;

	reset_allocation_count();
	int* i_ptr = new int[size];
	i_ptr[0] = -1;
	EXPECT_EQ(-1, i_ptr[0]);
	EXPECT_ALLOC(1, sizeof(int) * size);
	delete i_ptr;
}

TEST(AllocationCountingTest, GlobalNewArrayAllocationAccounted) {
	constexpr int size = 50;

	reset_allocation_count();
	int* i_ptr = ::new int[size];
	i_ptr[0] = -1;
	EXPECT_EQ(-1, i_ptr[0]);
	EXPECT_ALLOC(1, sizeof(int) * size);
	delete i_ptr;
}
#endif // SKIP_ALLOCATION_TESTS

int main(int argc, char **argv) {
#ifdef NDEBUG
	std::cout << "Running tets in Release configuration\n";
#else
	std::cout << "Running tets in Debug configuration\n";
#endif

	::testing::InitGoogleTest(&argc, argv);
	int ret = RUN_ALL_TESTS();
	return ret;
}
