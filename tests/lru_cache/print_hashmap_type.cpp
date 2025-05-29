#if !defined(LRU_CACHE_PRINT_HASH_MAP_IMPLEMENTATION) && !defined(SKIP_ALLOCATION_TESTS)
	#define LRU_CACHE_PRINT_HASH_MAP_IMPLEMENTATION
#endif

#include "hashmap.hpp"

#ifdef HASH_MAP_TYPE
	#undef HASH_MAP_TYPE
#endif
