#pragma once

#include "lrucache.hpp"

// std::unordered_map
#define STD_UNORDERED_MAP 10

// Abseil absl::flat_hash_map
#define ABSEIL_FLAT_HASH_MAP 20

// Tessil tsl::sparse_map
#define TESSIL_SPARSE_MAP 30
// Tessil tsl::robin_map
#define TESSIL_ROBIN_MAP 31
// Tessil tsl::hopscotch_map
#define TESSIL_HOPSCOTCH_MAP 32

// Ankerl ankerl::unordered_dense::map
#define ANKERL_UNORDERED_DENSE_MAP 40
// Ankerl ankerl::unordered_dense::segmented_map
#define ANKERL_UNORDERED_DENSE_SEGMENTED_MAP 41

// Set the default implementation
#ifndef LRU_CACHE_HASH_MAP_IMPLEMENTATION
	#define LRU_CACHE_HASH_MAP_IMPLEMENTATION STD_UNORDERED_MAP
#endif

// Use the correct headers and type for the selected implementation
#if LRU_CACHE_HASH_MAP_IMPLEMENTATION == STD_UNORDERED_MAP
	// std::unordered_map
	#include <unordered_map>
	#define HASH_MAP_TYPE std::unordered_map

	#ifdef LRU_CACHE_PRINT_HASH_MAP_IMPLEMENTATION
		#pragma message("Using std::unordered_map as the hashmap for the LRU cache")
	#endif
#elif LRU_CACHE_HASH_MAP_IMPLEMENTATION == ABSEIL_FLAT_HASH_MAP
	// Abseil absl::flat_hash_map
	#include "absl/container/flat_hash_map.h"
	#define HASH_MAP_TYPE absl::flat_hash_map

	#ifdef LRU_CACHE_PRINT_HASH_MAP_IMPLEMENTATION
		#pragma message("Using absl::flat_hash_map as the hashmap for the LRU cache")
	#endif
#elif LRU_CACHE_HASH_MAP_IMPLEMENTATION == TESSIL_SPARSE_MAP
	// Tessil tsl::sparse_map
	#include "tsl/sparse_map.h"
	#define HASH_MAP_TYPE tsl::sparse_map

	#ifdef LRU_CACHE_PRINT_HASH_MAP_IMPLEMENTATION
		#pragma message("Using tsl::sparse_map as the hashmap for the LRU cache")
	#endif
#elif LRU_CACHE_HASH_MAP_IMPLEMENTATION == TESSIL_ROBIN_MAP
	// Tessil tsl::robin_map
	#include "tsl/robin_map.h"
	#define HASH_MAP_TYPE tsl::robin_map

	#ifdef LRU_CACHE_PRINT_HASH_MAP_IMPLEMENTATION
		#pragma message("Using tsl::robin_map as the hashmap for the LRU cache")
	#endif
#elif LRU_CACHE_HASH_MAP_IMPLEMENTATION == TESSIL_HOPSCOTCH_MAP
	// Tessil tsl::hopscotch_map
	#include "tsl/hopscotch_map.h"
	#define HASH_MAP_TYPE tsl::hopscotch_map

	#ifdef LRU_CACHE_PRINT_HASH_MAP_IMPLEMENTATION
		#pragma message("Using tsl::hopscotch_map as the hashmap for the LRU cache")
	#endif
#elif LRU_CACHE_HASH_MAP_IMPLEMENTATION == ANKERL_UNORDERED_DENSE_MAP
	// Ankerl ankerl::unordered_dense::map
	#include "ankerl/unordered_dense.h"
	#define HASH_MAP_TYPE ankerl::unordered_dense::map

	#ifdef LRU_CACHE_PRINT_HASH_MAP_IMPLEMENTATION
		#pragma message("Using ankerl::unordered_dense::map as the hashmap for the LRU cache")
	#endif
#elif LRU_CACHE_HASH_MAP_IMPLEMENTATION == ANKERL_UNORDERED_DENSE_SEGMENTED_MAP
	// Ankerl ankerl::unordered_dense::segmented_map
	#include "ankerl/unordered_dense.h"
	#define HASH_MAP_TYPE ankerl::unordered_dense::segmented_map

	#ifdef LRU_CACHE_PRINT_HASH_MAP_IMPLEMENTATION
		#pragma message("Using ankerl::unordered_dense::segmented_map as the hashmap for the LRU cache")
	#endif
#else
	#ifdef VALUE_TO_STRING
		#undef VALUE_TO_STRING
	#endif
	#ifdef VALUE
		#undef VALUE
	#endif
	#define VALUE_TO_STRING(x) #x
	#define VALUE(x) VALUE_TO_STRING(x)

	#pragma message("LRU_CACHE_HASH_MAP_IMPLEMENTATION is set to " VALUE(LRU_CACHE_HASH_MAP_IMPLEMENTATION))
	#pragma message("Possible valiues are STD_UNORDERED_MAP(std::unordered_map), ABSEIL_FLAT_HASH_MAP(absl::flat_hash_map), TESSIL_SPARSE_MAP(tsl::sparse_map), TESSIL_ROBIN_MAP(tsl::robin_map), TESSIL_HOPSCOTCH_MAP(tsl::hopscotch_map), ANKERL_UNORDERED_DENSE_MAP(ankerl::unordered_dense::map), ANKERL_UNORDERED_DENSE_SEGMENTED_MAP(ankerl::unordered_dense::segmented_map)")
	#error "Unexpected value of LRU_CACHE_HASH_MAP_IMPLEMENTATION"
#endif
