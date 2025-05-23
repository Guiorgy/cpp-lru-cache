#include "lrucache.hpp"

// preallocate = false (default)
static_assert(std::is_constructible_v<guiorgy::lru_cache<int, int, 1>> == true);
static_assert(std::is_trivially_constructible_v<guiorgy::lru_cache<int, int, 1>> == false);
#if LRU_CACHE_HASH_MAP_IMPLEMENTATION == STL_UNORDERED_MAP
	static_assert(std::is_nothrow_constructible_v<guiorgy::lru_cache<int, int, 1>> == true);
#elif (LRU_CACHE_HASH_MAP_IMPLEMENTATION == STL_PMR_UNORDERED_MAP) || (LRU_CACHE_HASH_MAP_IMPLEMENTATION == ABSEIL_FLAT_HASH_MAP) || (LRU_CACHE_HASH_MAP_IMPLEMENTATION == TESSIL_SPARSE_MAP) || (LRU_CACHE_HASH_MAP_IMPLEMENTATION == ANKERL_UNORDERED_DENSE_MAP) || (LRU_CACHE_HASH_MAP_IMPLEMENTATION == ANKERL_UNORDERED_DENSE_SEGMENTED_MAP)
	static_assert(std::is_nothrow_constructible_v<guiorgy::lru_cache<int, int, 1>> == false);
#else
	#error "Unhandled case of LRU_CACHE_HASH_MAP_IMPLEMENTATION"
#endif

static_assert(std::is_default_constructible_v<guiorgy::lru_cache<int, int, 1>> == true);
static_assert(std::is_trivially_default_constructible_v<guiorgy::lru_cache<int, int, 1>> == false);
#if LRU_CACHE_HASH_MAP_IMPLEMENTATION == STL_UNORDERED_MAP
	static_assert(std::is_nothrow_default_constructible_v<guiorgy::lru_cache<int, int, 1>> == true);
#elif (LRU_CACHE_HASH_MAP_IMPLEMENTATION == STL_PMR_UNORDERED_MAP) || (LRU_CACHE_HASH_MAP_IMPLEMENTATION == ABSEIL_FLAT_HASH_MAP) || (LRU_CACHE_HASH_MAP_IMPLEMENTATION == TESSIL_SPARSE_MAP) || (LRU_CACHE_HASH_MAP_IMPLEMENTATION == ANKERL_UNORDERED_DENSE_MAP) || (LRU_CACHE_HASH_MAP_IMPLEMENTATION == ANKERL_UNORDERED_DENSE_SEGMENTED_MAP)
	static_assert(std::is_nothrow_default_constructible_v<guiorgy::lru_cache<int, int, 1>> == false);
#else
	#error "Unhandled case of LRU_CACHE_HASH_MAP_IMPLEMENTATION"
#endif

#if (LRU_CACHE_HASH_MAP_IMPLEMENTATION == STL_UNORDERED_MAP) || (LRU_CACHE_HASH_MAP_IMPLEMENTATION == ABSEIL_FLAT_HASH_MAP) || (LRU_CACHE_HASH_MAP_IMPLEMENTATION == TESSIL_SPARSE_MAP) || (LRU_CACHE_HASH_MAP_IMPLEMENTATION == ANKERL_UNORDERED_DENSE_MAP) || (LRU_CACHE_HASH_MAP_IMPLEMENTATION == ANKERL_UNORDERED_DENSE_SEGMENTED_MAP)
	static_assert(std::is_copy_constructible_v<guiorgy::lru_cache<int, int, 1>> == true);
#elif LRU_CACHE_HASH_MAP_IMPLEMENTATION == STL_PMR_UNORDERED_MAP
	static_assert(std::is_copy_constructible_v<guiorgy::lru_cache<int, int, 1>> == false);
#else
	#error "Unhandled case of LRU_CACHE_HASH_MAP_IMPLEMENTATION"
#endif
static_assert(std::is_trivially_copy_constructible_v<guiorgy::lru_cache<int, int, 1>> == false);
static_assert(std::is_nothrow_copy_constructible_v<guiorgy::lru_cache<int, int, 1>> == false);

#if (LRU_CACHE_HASH_MAP_IMPLEMENTATION == STL_UNORDERED_MAP) || (LRU_CACHE_HASH_MAP_IMPLEMENTATION == ABSEIL_FLAT_HASH_MAP) || (LRU_CACHE_HASH_MAP_IMPLEMENTATION == TESSIL_SPARSE_MAP) || (LRU_CACHE_HASH_MAP_IMPLEMENTATION == ANKERL_UNORDERED_DENSE_MAP) || (LRU_CACHE_HASH_MAP_IMPLEMENTATION == ANKERL_UNORDERED_DENSE_SEGMENTED_MAP)
	static_assert(std::is_move_constructible_v<guiorgy::lru_cache<int, int, 1>> == true);
#elif LRU_CACHE_HASH_MAP_IMPLEMENTATION == STL_PMR_UNORDERED_MAP
	static_assert(std::is_move_constructible_v<guiorgy::lru_cache<int, int, 1>> == false);
#else
	#error "Unhandled case of LRU_CACHE_HASH_MAP_IMPLEMENTATION"
#endif
static_assert(std::is_trivially_move_constructible_v<guiorgy::lru_cache<int, int, 1>> == false);
#if (LRU_CACHE_HASH_MAP_IMPLEMENTATION == STL_UNORDERED_MAP) || (LRU_CACHE_HASH_MAP_IMPLEMENTATION == ABSEIL_FLAT_HASH_MAP) || (LRU_CACHE_HASH_MAP_IMPLEMENTATION == TESSIL_SPARSE_MAP) || (LRU_CACHE_HASH_MAP_IMPLEMENTATION == ANKERL_UNORDERED_DENSE_MAP) || (LRU_CACHE_HASH_MAP_IMPLEMENTATION == ANKERL_UNORDERED_DENSE_SEGMENTED_MAP)
	static_assert(std::is_nothrow_move_constructible_v<guiorgy::lru_cache<int, int, 1>> == true);
#elif LRU_CACHE_HASH_MAP_IMPLEMENTATION == STL_PMR_UNORDERED_MAP
	static_assert(std::is_nothrow_move_constructible_v<guiorgy::lru_cache<int, int, 1>> == false);
#else
	#error "Unhandled case of LRU_CACHE_HASH_MAP_IMPLEMENTATION"
#endif

// preallocate = true
static_assert(std::is_constructible_v<guiorgy::lru_cache<int, int, 1, true>> == true);
static_assert(std::is_trivially_constructible_v<guiorgy::lru_cache<int, int, 1, true>> == false);
static_assert(std::is_nothrow_constructible_v<guiorgy::lru_cache<int, int, 1, true>> == false);

// static_assert(std::is_default_constructible_v<guiorgy::lru_cache<int, int, 1, true>> == false); // This fails?!
static_assert(std::is_trivially_default_constructible_v<guiorgy::lru_cache<int, int, 1, true>> == false);
static_assert(std::is_nothrow_default_constructible_v<guiorgy::lru_cache<int, int, 1, true>> == false);

#if (LRU_CACHE_HASH_MAP_IMPLEMENTATION == STL_UNORDERED_MAP) || (LRU_CACHE_HASH_MAP_IMPLEMENTATION == ABSEIL_FLAT_HASH_MAP) || (LRU_CACHE_HASH_MAP_IMPLEMENTATION == TESSIL_SPARSE_MAP) || (LRU_CACHE_HASH_MAP_IMPLEMENTATION == ANKERL_UNORDERED_DENSE_MAP) || (LRU_CACHE_HASH_MAP_IMPLEMENTATION == ANKERL_UNORDERED_DENSE_SEGMENTED_MAP)
	static_assert(std::is_copy_constructible_v<guiorgy::lru_cache<int, int, 1, true>> == true);
#elif LRU_CACHE_HASH_MAP_IMPLEMENTATION == STL_PMR_UNORDERED_MAP
	static_assert(std::is_copy_constructible_v<guiorgy::lru_cache<int, int, 1, true>> == false);
#else
	#error "Unhandled case of LRU_CACHE_HASH_MAP_IMPLEMENTATION"
#endif
static_assert(std::is_trivially_copy_constructible_v<guiorgy::lru_cache<int, int, 1, true>> == false);
static_assert(std::is_nothrow_copy_constructible_v<guiorgy::lru_cache<int, int, 1, true>> == false);

#if (LRU_CACHE_HASH_MAP_IMPLEMENTATION == STL_UNORDERED_MAP) || (LRU_CACHE_HASH_MAP_IMPLEMENTATION == ABSEIL_FLAT_HASH_MAP) || (LRU_CACHE_HASH_MAP_IMPLEMENTATION == TESSIL_SPARSE_MAP) || (LRU_CACHE_HASH_MAP_IMPLEMENTATION == ANKERL_UNORDERED_DENSE_MAP) || (LRU_CACHE_HASH_MAP_IMPLEMENTATION == ANKERL_UNORDERED_DENSE_SEGMENTED_MAP)
	static_assert(std::is_move_constructible_v<guiorgy::lru_cache<int, int, 1, true>> == true);
#elif LRU_CACHE_HASH_MAP_IMPLEMENTATION == STL_PMR_UNORDERED_MAP
	static_assert(std::is_move_constructible_v<guiorgy::lru_cache<int, int, 1, true>> == false);
#else
	#error "Unhandled case of LRU_CACHE_HASH_MAP_IMPLEMENTATION"
#endif
static_assert(std::is_trivially_move_constructible_v<guiorgy::lru_cache<int, int, 1, true>> == false);
#if (LRU_CACHE_HASH_MAP_IMPLEMENTATION == STL_UNORDERED_MAP) || (LRU_CACHE_HASH_MAP_IMPLEMENTATION == ABSEIL_FLAT_HASH_MAP) || (LRU_CACHE_HASH_MAP_IMPLEMENTATION == TESSIL_SPARSE_MAP) || (LRU_CACHE_HASH_MAP_IMPLEMENTATION == ANKERL_UNORDERED_DENSE_MAP) || (LRU_CACHE_HASH_MAP_IMPLEMENTATION == ANKERL_UNORDERED_DENSE_SEGMENTED_MAP)
	static_assert(std::is_nothrow_move_constructible_v<guiorgy::lru_cache<int, int, 1, true>> == true);
#elif LRU_CACHE_HASH_MAP_IMPLEMENTATION == STL_PMR_UNORDERED_MAP
	static_assert(std::is_nothrow_move_constructible_v<guiorgy::lru_cache<int, int, 1, true>> == false);
#else
	#error "Unhandled case of LRU_CACHE_HASH_MAP_IMPLEMENTATION"
#endif
