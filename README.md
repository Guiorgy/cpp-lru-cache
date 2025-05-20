# cpp-lru-cache

> ⚠️ **This implementation is NOT thread safe!**

An LRU (Least Recently Used) cache for c++ based on hashmap and a packed linkedlist based on vector. The library is header only, a simple test and example are included.

> ⚠️ *For a simpler implementation that only includes standard STL containers and very little of its own logics (and probably more reliable) see the stl-only branch.*

### Example

```c++
/**Creates cache with maximum size of three. When the 
   size in achieved every next element will replace the 
   least recently used one */
lru_cache<std::string, std::string, 3> cache;

/**Preallocates the necessary memory to avoid reallocations.
   Alternatively you can set the fourth template argument
   (bool preallocate) to true during declaration */
cache.reserve();

cache.put("one", "one");
cache.put("two", "two");

const std::string& from_cache_1 = cache.get("two").value();

/**For objects too expensive or impossible to copy
   you can get them by reference, however, no guarantees
   are given about the underlying object lifetime
   when modifying the cache, so use with caution */
const std::string& from_cache_2 = cache.get_ref("two")->get();
```

### Change the underlying hashmap implementation

The underlying hashmap implementation can be changed from the default STL `std::unordered_map` to the following:

- [Abseil](https://github.com/abseil/abseil-cpp) `absl::flat_hash_map` (`ABSEIL_FLAT_HASH_MAP`)
- [Tessil](https://github.com/Tessil/sparse-map) `tsl::sparse_map` (`TESSIL_SPARSE_MAP`)
- [Ankerl](https://github.com/martinus/unordered_dense) `ankerl::unordered_dense::map` (`ANKERL_UNORDERED_DENSE_MAP`) and `ankerl::unordered_dense::segmented_map` (`ANKERL_UNORDERED_DENSE_SEGMENTED_MAP`)

To switch to the desired implementation, set the `LRU_CACHE_HASH_MAP_IMPLEMENTATION` compiler definition to the desired value. For example:

- Compiler

    ```shell
    g++ ... -DLRU_CACHE_HASH_MAP_IMPLEMENTATION=ABSEIL_FLAT_HASH_MAP
    ```

- Source

    ```c++
    #define LRU_CACHE_HASH_MAP_IMPLEMENTATION ABSEIL_FLAT_HASH_MAP
    #include "lrucache.hpp"
    ```

To decide which implementation to use, check out the amazing round of [benchmarks](https://martin.ankerl.com/2022/08/27/hashmap-bench-01/) for many map implementations done by Martin Leitner-Ankerl.

### How to run tests

```shell
cd build
cmake ..
# cmake -DCMAKE_BUILD_TYPE=Debug -DABSEIL=OFF -DTESSIL=OFF -DANKERL=OFF -DANKERL_SEG=OFF ..
make test
# make sanitize
# ./full-test.sh
```
