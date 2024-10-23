#include "lrucache.hpp"
#include "gtest/gtest.h"

constexpr int NUM_OF_TEST1_RECORDS = 100;
constexpr int NUM_OF_TEST2_RECORDS = 100;
constexpr int TEST2_CACHE_CAPACITY = 50;

TEST(CacheTest, SimplePut) {
    lru_cache<int, int, 1> cache_lru;
    cache_lru.put(7, 777);

    EXPECT_TRUE(cache_lru.exists(7));
    EXPECT_EQ(777, cache_lru.get(7).value());
    EXPECT_EQ(1, cache_lru.size());
}

TEST(CacheTest, MissingValue) {
    lru_cache<int, int, 1> cache_lru;
    auto cached = cache_lru.get(7);

    EXPECT_FALSE(cached.has_value());
}

TEST(CacheTest1, KeepsAllValuesWithinCapacity) {
    lru_cache<int, int, TEST2_CACHE_CAPACITY> cache_lru;

    for (int i = 0; i < NUM_OF_TEST2_RECORDS; ++i) {
        cache_lru.put(i, i);
    }

    for (int i = 0; i < NUM_OF_TEST2_RECORDS - TEST2_CACHE_CAPACITY; ++i) {
        EXPECT_FALSE(cache_lru.exists(i));
    }

    for (int i = NUM_OF_TEST2_RECORDS - TEST2_CACHE_CAPACITY; i < NUM_OF_TEST2_RECORDS; ++i) {
        EXPECT_TRUE(cache_lru.exists(i));

        auto cached = cache_lru.get(i);
        EXPECT_TRUE(cached.has_value());
        EXPECT_EQ(i, cached.value());
    }

    size_t size = cache_lru.size();
    EXPECT_EQ(TEST2_CACHE_CAPACITY, size);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
    return ret;
}
