#pragma once

/* Define possible implementations and store old definitions to restore later in case of a conflict */
// STL std::unordered_map
#ifdef STL_UNORDERED_MAP
	#define GUIORGY_LRU_CACHE_STL_UNORDERED_MAP_BEFORE STL_UNORDERED_MAP
	#undef STL_UNORDERED_MAP
#endif
#define STL_UNORDERED_MAP 10
// STL std::pmr::unordered_map with std::pmr::unsynchronized_pool_resource
#ifdef STL_PMR_UNORDERED_MAP
	#define GUIORGY_LRU_CACHE_STL_PMR_UNORDERED_MAP_BEFORE STL_PMR_UNORDERED_MAP
	#undef STL_PMR_UNORDERED_MAP
#endif
#define STL_PMR_UNORDERED_MAP 11

// Abseil absl::flat_hash_map
#ifdef ABSEIL_FLAT_HASH_MAP
	#define GUIORGY_LRU_CACHE_ABSEIL_FLAT_HASH_MAP_BEFORE ABSEIL_FLAT_HASH_MAP
	#undef ABSEIL_FLAT_HASH_MAP
#endif
#define ABSEIL_FLAT_HASH_MAP 20

// Tessil tsl::sparse_map
#ifdef TESSIL_SPARSE_MAP
	#define GUIORGY_LRU_CACHE_TESSIL_SPARSE_MAP_BEFORE TESSIL_SPARSE_MAP
	#undef TESSIL_SPARSE_MAP
#endif
#define TESSIL_SPARSE_MAP 30
// Tessil tsl::robin_map
#ifdef TESSIL_ROBIN_MAP
	#define GUIORGY_LRU_CACHE_TESSIL_ROBIN_MAP_BEFORE TESSIL_ROBIN_MAP
	#undef TESSIL_ROBIN_MAP
#endif
#define TESSIL_ROBIN_MAP 31
// Tessil tsl::hopscotch_map
#ifdef TESSIL_HOPSCOTCH_MAP
	#define GUIORGY_LRU_CACHE_TESSIL_HOPSCOTCH_MAP_BEFORE TESSIL_HOPSCOTCH_MAP
	#undef TESSIL_HOPSCOTCH_MAP
#endif
#define TESSIL_HOPSCOTCH_MAP 32

// Ankerl ankerl::unordered_dense::map
#ifdef ANKERL_UNORDERED_DENSE_MAP
	#define GUIORGY_LRU_CACHE_ANKERL_UNORDERED_DENSE_MAP_BEFORE ANKERL_UNORDERED_DENSE_MAP
	#undef ANKERL_UNORDERED_DENSE_MAP
#endif
#define ANKERL_UNORDERED_DENSE_MAP 40
// Ankerl ankerl::unordered_dense::segmented_map
#ifdef ANKERL_UNORDERED_DENSE_SEGMENTED_MAP
	#define GUIORGY_LRU_CACHE_ANKERL_UNORDERED_DENSE_SEGMENTED_MAP_BEFORE ANKERL_UNORDERED_DENSE_SEGMENTED_MAP
	#undef ANKERL_UNORDERED_DENSE_SEGMENTED_MAP
#endif
#define ANKERL_UNORDERED_DENSE_SEGMENTED_MAP 41

// Set the default implementation
#ifndef LRU_CACHE_HASH_MAP_IMPLEMENTATION
	#define LRU_CACHE_HASH_MAP_IMPLEMENTATION STL_UNORDERED_MAP
#endif

// Use the correct headers for the selected implementation
#if LRU_CACHE_HASH_MAP_IMPLEMENTATION == STL_UNORDERED_MAP
	// STL std::unordered_map
	#include <unordered_map>

	#ifdef LRU_CACHE_PRINT_HASH_MAP_IMPLEMENTATION
		#pragma message("Using std::unordered_map as the hashmap for the LRU cache")
	#endif
#elif LRU_CACHE_HASH_MAP_IMPLEMENTATION == STL_PMR_UNORDERED_MAP
	// STL std::pmr::unordered_map with std::pmr::unsynchronized_pool_resource
	#include <memory_resource>
	#include <unordered_map>

	#ifdef LRU_CACHE_PRINT_HASH_MAP_IMPLEMENTATION
		#pragma message("Using std::pmr::unordered_map as the hashmap for the LRU cache")
	#endif
#elif LRU_CACHE_HASH_MAP_IMPLEMENTATION == ABSEIL_FLAT_HASH_MAP
	// Abseil absl::flat_hash_map
	#include "absl/container/flat_hash_map.h"

	#ifdef LRU_CACHE_PRINT_HASH_MAP_IMPLEMENTATION
		#pragma message("Using absl::flat_hash_map as the hashmap for the LRU cache")
	#endif
#elif LRU_CACHE_HASH_MAP_IMPLEMENTATION == TESSIL_SPARSE_MAP
	// Tessil tsl::sparse_map
	#include "tsl/sparse_map.h"

	#ifdef LRU_CACHE_PRINT_HASH_MAP_IMPLEMENTATION
		#pragma message("Using tsl::sparse_map as the hashmap for the LRU cache")
	#endif
#elif LRU_CACHE_HASH_MAP_IMPLEMENTATION == TESSIL_ROBIN_MAP
	// Tessil tsl::robin_map
	#include "tsl/robin_map.h"

	#ifdef LRU_CACHE_PRINT_HASH_MAP_IMPLEMENTATION
		#pragma message("Using tsl::robin_map as the hashmap for the LRU cache")
	#endif
#elif LRU_CACHE_HASH_MAP_IMPLEMENTATION == TESSIL_HOPSCOTCH_MAP
	// Tessil tsl::hopscotch_map
	#include "tsl/hopscotch_map.h"

	#ifdef LRU_CACHE_PRINT_HASH_MAP_IMPLEMENTATION
		#pragma message("Using tsl::hopscotch_map as the hashmap for the LRU cache")
	#endif
#elif LRU_CACHE_HASH_MAP_IMPLEMENTATION == ANKERL_UNORDERED_DENSE_MAP
	// Ankerl ankerl::unordered_dense::map
	#include "ankerl/unordered_dense.h"

	#ifdef LRU_CACHE_PRINT_HASH_MAP_IMPLEMENTATION
		#pragma message("Using ankerl::unordered_dense::map as the hashmap for the LRU cache")
	#endif
#elif LRU_CACHE_HASH_MAP_IMPLEMENTATION == ANKERL_UNORDERED_DENSE_SEGMENTED_MAP
	// Ankerl ankerl::unordered_dense::segmented_map
	#include "ankerl/unordered_dense.h"

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
	#pragma message("Possible valiues are STL_UNORDERED_MAP(std::unordered_map), STL_PMR_UNORDERED_MAP(std::pmr::unordered_map), ABSEIL_FLAT_HASH_MAP(absl::flat_hash_map), TESSIL_SPARSE_MAP(tsl::sparse_map), TESSIL_ROBIN_MAP(tsl::robin_map), TESSIL_HOPSCOTCH_MAP(tsl::hopscotch_map), ANKERL_UNORDERED_DENSE_MAP(ankerl::unordered_dense::map), ANKERL_UNORDERED_DENSE_SEGMENTED_MAP(ankerl::unordered_dense::segmented_map)")
	#error "Unexpected value of LRU_CACHE_HASH_MAP_IMPLEMENTATION"
#endif



/* Restore old definitions if they were already defined */
// STL std::unordered_map
#ifdef GUIORGY_LRU_CACHE_STL_UNORDERED_MAP_BEFORE
	#undef STL_UNORDERED_MAP
	#define STL_UNORDERED_MAP GUIORGY_LRU_CACHE_STL_UNORDERED_MAP_BEFORE
	#undef GUIORGY_LRU_CACHE_STL_UNORDERED_MAP_BEFORE
#endif
// STL std::pmr::unordered_map with std::pmr::unsynchronized_pool_resource
#ifdef GUIORGY_LRU_CACHE_STL_PMR_UNORDERED_MAP_BEFORE
	#undef STL_MPR_UNORDERED_MAP
	#define STL_MPR_UNORDERED_MAP GUIORGY_LRU_CACHE_STL_PMR_UNORDERED_MAP_BEFORE
	#undef GUIORGY_LRU_CACHE_STL_PMR_UNORDERED_MAP_BEFORE
#endif

// STL std::unordered_map
#ifdef GUIORGY_LRU_CACHE_ABSEIL_FLAT_HASH_MAP_BEFORE
	#undef ABSEIL_FLAT_HASH_MAP
	#define ABSEIL_FLAT_HASH_MAP GUIORGY_LRU_CACHE_ABSEIL_FLAT_HASH_MAP_BEFORE
	#undef GUIORGY_LRU_CACHE_ABSEIL_FLAT_HASH_MAP_BEFORE
#endif

// Tessil tsl::sparse_map
#ifdef GUIORGY_LRU_CACHE_TESSIL_SPARSE_MAP_BEFORE
	#undef TESSIL_SPARSE_MAP
	#define TESSIL_SPARSE_MAP GUIORGY_LRU_CACHE_TESSIL_SPARSE_MAP_BEFORE
	#undef GUIORGY_LRU_CACHE_TESSIL_SPARSE_MAP_BEFORE
#endif
// Tessil tsl::robin_map
#ifdef GUIORGY_LRU_CACHE_TESSIL_ROBIN_MAP_BEFORE
	#undef TESSIL_ROBIN_MAP
	#define TESSIL_ROBIN_MAP GUIORGY_LRU_CACHE_TESSIL_ROBIN_MAP_BEFORE
	#undef GUIORGY_LRU_CACHE_TESSIL_ROBIN_MAP_BEFORE
#endif
// Tessil tsl::hopscotch_map
#ifdef GUIORGY_LRU_CACHE_TESSIL_HOPSCOTCH_MAP_BEFORE
	#undef TESSIL_HOPSCOTCH_MAP
	#define TESSIL_HOPSCOTCH_MAP GUIORGY_LRU_CACHE_TESSIL_HOPSCOTCH_MAP_BEFORE
	#undef GUIORGY_LRU_CACHE_TESSIL_HOPSCOTCH_MAP_BEFORE
#endif

// Ankerl ankerl::unordered_dense
#ifdef GUIORGY_LRU_CACHE_ANKERL_UNORDERED_DENSE_MAP_BEFORE
	#undef ANKERL_UNORDERED_DENSE_MAP
	#define ANKERL_UNORDERED_DENSE_MAP GUIORGY_LRU_CACHE_ANKERL_UNORDERED_DENSE_MAP_BEFORE
	#undef GUIORGY_LRU_CACHE_ANKERL_UNORDERED_DENSE_MAP_BEFORE
#endif
// Ankerl ankerl::unordered_dense::segmented_map
#ifdef GUIORGY_LRU_CACHE_ANKERL_UNORDERED_DENSE_SEGMENTED_MAP_BEFORE
	#undef ANKERL_UNORDERED_DENSE_SEGMENTED_MAP
	#define ANKERL_UNORDERED_DENSE_SEGMENTED_MAP GUIORGY_LRU_CACHE_ANKERL_UNORDERED_DENSE_SEGMENTED_MAP_BEFORE
	#undef GUIORGY_LRU_CACHE_ANKERL_UNORDERED_DENSE_SEGMENTED_MAP_BEFORE
#endif
