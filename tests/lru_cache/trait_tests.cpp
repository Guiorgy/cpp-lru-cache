#include "lrucache.hpp"

#define NO_PREALLOCATE guiorgy::lru_cache<int, int, 1, false>
#define PREALLOCATE guiorgy::lru_cache<int, int, 1, true>

#if LRU_CACHE_HASH_MAP_IMPLEMENTATION == STL_UNORDERED_MAP
	// preallocate = false (default)
	static_assert(std::is_constructible_v<NO_PREALLOCATE> == true);
	static_assert(std::is_trivially_constructible_v<NO_PREALLOCATE> == false);
	static_assert(std::is_nothrow_constructible_v<NO_PREALLOCATE> == true);

	static_assert(std::is_default_constructible_v<NO_PREALLOCATE> == true);
	static_assert(std::is_trivially_default_constructible_v<NO_PREALLOCATE> == false);
	static_assert(std::is_nothrow_default_constructible_v<NO_PREALLOCATE> == true);

	static_assert(std::is_copy_constructible_v<NO_PREALLOCATE> == true);
	static_assert(std::is_trivially_copy_constructible_v<NO_PREALLOCATE> == false);
	static_assert(std::is_nothrow_copy_constructible_v<NO_PREALLOCATE> == false);

	static_assert(std::is_move_constructible_v<NO_PREALLOCATE> == true);
	static_assert(std::is_trivially_move_constructible_v<NO_PREALLOCATE> == false);
	static_assert(std::is_nothrow_move_constructible_v<NO_PREALLOCATE> == true);

	// preallocate = true
	static_assert(std::is_constructible_v<PREALLOCATE> == true);
	static_assert(std::is_trivially_constructible_v<PREALLOCATE> == false);
	static_assert(std::is_nothrow_constructible_v<PREALLOCATE> == false);

	static_assert(std::is_default_constructible_v<PREALLOCATE> == true);
	static_assert(std::is_trivially_default_constructible_v<PREALLOCATE> == false);
	static_assert(std::is_nothrow_default_constructible_v<PREALLOCATE> == false);

	static_assert(std::is_copy_constructible_v<PREALLOCATE> == true);
	static_assert(std::is_trivially_copy_constructible_v<PREALLOCATE> == false);
	static_assert(std::is_nothrow_copy_constructible_v<PREALLOCATE> == false);

	static_assert(std::is_move_constructible_v<PREALLOCATE> == true);
	static_assert(std::is_trivially_move_constructible_v<PREALLOCATE> == false);
	static_assert(std::is_nothrow_move_constructible_v<PREALLOCATE> == true);
#elif LRU_CACHE_HASH_MAP_IMPLEMENTATION == STL_PMR_UNORDERED_MAP
	// preallocate = false (default)
	static_assert(std::is_constructible_v<NO_PREALLOCATE> == true);
	static_assert(std::is_trivially_constructible_v<NO_PREALLOCATE> == false);
	static_assert(std::is_nothrow_constructible_v<NO_PREALLOCATE> == false);

	static_assert(std::is_default_constructible_v<NO_PREALLOCATE> == true);
	static_assert(std::is_trivially_default_constructible_v<NO_PREALLOCATE> == false);
	static_assert(std::is_nothrow_default_constructible_v<NO_PREALLOCATE> == false);

	static_assert(std::is_copy_constructible_v<NO_PREALLOCATE> == false);
	static_assert(std::is_trivially_copy_constructible_v<NO_PREALLOCATE> == false);
	static_assert(std::is_nothrow_copy_constructible_v<NO_PREALLOCATE> == false);

	static_assert(std::is_move_constructible_v<NO_PREALLOCATE> == false);
	static_assert(std::is_trivially_move_constructible_v<NO_PREALLOCATE> == false);
	static_assert(std::is_nothrow_move_constructible_v<NO_PREALLOCATE> == false);

	// preallocate = true
	static_assert(std::is_constructible_v<PREALLOCATE> == true);
	static_assert(std::is_trivially_constructible_v<PREALLOCATE> == false);
	static_assert(std::is_nothrow_constructible_v<PREALLOCATE> == false);

	static_assert(std::is_default_constructible_v<PREALLOCATE> == true);
	static_assert(std::is_trivially_default_constructible_v<PREALLOCATE> == false);
	static_assert(std::is_nothrow_default_constructible_v<PREALLOCATE> == false);

	static_assert(std::is_copy_constructible_v<PREALLOCATE> == false);
	static_assert(std::is_trivially_copy_constructible_v<PREALLOCATE> == false);
	static_assert(std::is_nothrow_copy_constructible_v<PREALLOCATE> == false);

	static_assert(std::is_move_constructible_v<PREALLOCATE> == false);
	static_assert(std::is_trivially_move_constructible_v<PREALLOCATE> == false);
	static_assert(std::is_nothrow_move_constructible_v<PREALLOCATE> == false);
#elif LRU_CACHE_HASH_MAP_IMPLEMENTATION == ABSEIL_FLAT_HASH_MAP
	// preallocate = false (default)
	static_assert(std::is_constructible_v<NO_PREALLOCATE> == true);
	static_assert(std::is_trivially_constructible_v<NO_PREALLOCATE> == false);
	static_assert(std::is_nothrow_constructible_v<NO_PREALLOCATE> == false);

	static_assert(std::is_default_constructible_v<NO_PREALLOCATE> == true);
	static_assert(std::is_trivially_default_constructible_v<NO_PREALLOCATE> == false);
	static_assert(std::is_nothrow_default_constructible_v<NO_PREALLOCATE> == false);

	static_assert(std::is_copy_constructible_v<NO_PREALLOCATE> == true);
	static_assert(std::is_trivially_copy_constructible_v<NO_PREALLOCATE> == false);
	static_assert(std::is_nothrow_copy_constructible_v<NO_PREALLOCATE> == false);

	static_assert(std::is_move_constructible_v<NO_PREALLOCATE> == true);
	static_assert(std::is_trivially_move_constructible_v<NO_PREALLOCATE> == false);
	static_assert(std::is_nothrow_move_constructible_v<NO_PREALLOCATE> == true);

	// preallocate = true
	static_assert(std::is_constructible_v<PREALLOCATE> == true);
	static_assert(std::is_trivially_constructible_v<PREALLOCATE> == false);
	static_assert(std::is_nothrow_constructible_v<PREALLOCATE> == false);

	static_assert(std::is_default_constructible_v<PREALLOCATE> == true);
	static_assert(std::is_trivially_default_constructible_v<PREALLOCATE> == false);
	static_assert(std::is_nothrow_default_constructible_v<PREALLOCATE> == false);

	static_assert(std::is_copy_constructible_v<PREALLOCATE> == true);
	static_assert(std::is_trivially_copy_constructible_v<PREALLOCATE> == false);
	static_assert(std::is_nothrow_copy_constructible_v<PREALLOCATE> == false);

	static_assert(std::is_move_constructible_v<PREALLOCATE> == true);
	static_assert(std::is_trivially_move_constructible_v<PREALLOCATE> == false);
	static_assert(std::is_nothrow_move_constructible_v<PREALLOCATE> == true);
#elif LRU_CACHE_HASH_MAP_IMPLEMENTATION == TESSIL_SPARSE_MAP
	// preallocate = false (default)
	static_assert(std::is_constructible_v<NO_PREALLOCATE> == true);
	static_assert(std::is_trivially_constructible_v<NO_PREALLOCATE> == false);
	static_assert(std::is_nothrow_constructible_v<NO_PREALLOCATE> == false);

	static_assert(std::is_default_constructible_v<NO_PREALLOCATE> == true);
	static_assert(std::is_trivially_default_constructible_v<NO_PREALLOCATE> == false);
	static_assert(std::is_nothrow_default_constructible_v<NO_PREALLOCATE> == false);

	static_assert(std::is_copy_constructible_v<NO_PREALLOCATE> == true);
	static_assert(std::is_trivially_copy_constructible_v<NO_PREALLOCATE> == false);
	static_assert(std::is_nothrow_copy_constructible_v<NO_PREALLOCATE> == false);

	static_assert(std::is_move_constructible_v<NO_PREALLOCATE> == true);
	static_assert(std::is_trivially_move_constructible_v<NO_PREALLOCATE> == false);
	static_assert(std::is_nothrow_move_constructible_v<NO_PREALLOCATE> == true);

	// preallocate = true
	static_assert(std::is_constructible_v<PREALLOCATE> == true);
	static_assert(std::is_trivially_constructible_v<PREALLOCATE> == false);
	static_assert(std::is_nothrow_constructible_v<PREALLOCATE> == false);

	static_assert(std::is_default_constructible_v<PREALLOCATE> == true);
	static_assert(std::is_trivially_default_constructible_v<PREALLOCATE> == false);
	static_assert(std::is_nothrow_default_constructible_v<PREALLOCATE> == false);

	static_assert(std::is_copy_constructible_v<PREALLOCATE> == true);
	static_assert(std::is_trivially_copy_constructible_v<PREALLOCATE> == false);
	static_assert(std::is_nothrow_copy_constructible_v<PREALLOCATE> == false);

	static_assert(std::is_move_constructible_v<PREALLOCATE> == true);
	static_assert(std::is_trivially_move_constructible_v<PREALLOCATE> == false);
	static_assert(std::is_nothrow_move_constructible_v<PREALLOCATE> == true);
#elif LRU_CACHE_HASH_MAP_IMPLEMENTATION == TESSIL_ROBIN_MAP
	#error "TODO"
#elif LRU_CACHE_HASH_MAP_IMPLEMENTATION == TESSIL_HOPSCOTCH_MAP
	#error "TODO"
#elif LRU_CACHE_HASH_MAP_IMPLEMENTATION == ANKERL_UNORDERED_DENSE_MAP
	// preallocate = false (default)
	static_assert(std::is_constructible_v<NO_PREALLOCATE> == true);
	static_assert(std::is_trivially_constructible_v<NO_PREALLOCATE> == false);
	static_assert(std::is_nothrow_constructible_v<NO_PREALLOCATE> == false);

	static_assert(std::is_default_constructible_v<NO_PREALLOCATE> == true);
	static_assert(std::is_trivially_default_constructible_v<NO_PREALLOCATE> == false);
	static_assert(std::is_nothrow_default_constructible_v<NO_PREALLOCATE> == false);

	static_assert(std::is_copy_constructible_v<NO_PREALLOCATE> == true);
	static_assert(std::is_trivially_copy_constructible_v<NO_PREALLOCATE> == false);
	static_assert(std::is_nothrow_copy_constructible_v<NO_PREALLOCATE> == false);

	static_assert(std::is_move_constructible_v<NO_PREALLOCATE> == true);
	static_assert(std::is_trivially_move_constructible_v<NO_PREALLOCATE> == false);
	static_assert(std::is_nothrow_move_constructible_v<NO_PREALLOCATE> == true);

	// preallocate = true
	static_assert(std::is_constructible_v<PREALLOCATE> == true);
	static_assert(std::is_trivially_constructible_v<PREALLOCATE> == false);
	static_assert(std::is_nothrow_constructible_v<PREALLOCATE> == false);

	static_assert(std::is_default_constructible_v<PREALLOCATE> == true);
	static_assert(std::is_trivially_default_constructible_v<PREALLOCATE> == false);
	static_assert(std::is_nothrow_default_constructible_v<PREALLOCATE> == false);

	static_assert(std::is_copy_constructible_v<PREALLOCATE> == true);
	static_assert(std::is_trivially_copy_constructible_v<PREALLOCATE> == false);
	static_assert(std::is_nothrow_copy_constructible_v<PREALLOCATE> == false);

	static_assert(std::is_move_constructible_v<PREALLOCATE> == true);
	static_assert(std::is_trivially_move_constructible_v<PREALLOCATE> == false);
	static_assert(std::is_nothrow_move_constructible_v<PREALLOCATE> == true);
#elif LRU_CACHE_HASH_MAP_IMPLEMENTATION == ANKERL_UNORDERED_DENSE_SEGMENTED_MAP
	// preallocate = false (default)
	static_assert(std::is_constructible_v<NO_PREALLOCATE> == true);
	static_assert(std::is_trivially_constructible_v<NO_PREALLOCATE> == false);
	static_assert(std::is_nothrow_constructible_v<NO_PREALLOCATE> == false);

	static_assert(std::is_default_constructible_v<NO_PREALLOCATE> == true);
	static_assert(std::is_trivially_default_constructible_v<NO_PREALLOCATE> == false);
	static_assert(std::is_nothrow_default_constructible_v<NO_PREALLOCATE> == false);

	static_assert(std::is_copy_constructible_v<NO_PREALLOCATE> == true);
	static_assert(std::is_trivially_copy_constructible_v<NO_PREALLOCATE> == false);
	static_assert(std::is_nothrow_copy_constructible_v<NO_PREALLOCATE> == false);

	static_assert(std::is_move_constructible_v<NO_PREALLOCATE> == true);
	static_assert(std::is_trivially_move_constructible_v<NO_PREALLOCATE> == false);
	static_assert(std::is_nothrow_move_constructible_v<NO_PREALLOCATE> == true);

	// preallocate = true
	static_assert(std::is_constructible_v<PREALLOCATE> == true);
	static_assert(std::is_trivially_constructible_v<PREALLOCATE> == false);
	static_assert(std::is_nothrow_constructible_v<PREALLOCATE> == false);

	static_assert(std::is_default_constructible_v<PREALLOCATE> == true);
	static_assert(std::is_trivially_default_constructible_v<PREALLOCATE> == false);
	static_assert(std::is_nothrow_default_constructible_v<PREALLOCATE> == false);

	static_assert(std::is_copy_constructible_v<PREALLOCATE> == true);
	static_assert(std::is_trivially_copy_constructible_v<PREALLOCATE> == false);
	static_assert(std::is_nothrow_copy_constructible_v<PREALLOCATE> == false);

	static_assert(std::is_move_constructible_v<PREALLOCATE> == true);
	static_assert(std::is_trivially_move_constructible_v<PREALLOCATE> == false);
	static_assert(std::is_nothrow_move_constructible_v<PREALLOCATE> == true);
#else
	#error "Unhandled case of LRU_CACHE_HASH_MAP_IMPLEMENTATION"
#endif
