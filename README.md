# cpp-lru-cache

## ⚠️⚠️⚠️ Currently WIP and experimental ⚠️⚠️⚠️

> ⚠️ **This implementation is NOT thread safe!**

An LRU (Least Recently Used) cache for C++ 17 based on hashmap and a packed linkedlist based on vector. The library is header only, a simple test and example are included.

> ⚠️ *For a simpler implementation that only includes standard STD containers and very little of its own logics (and probably more reliable) see the std-only branch.*

### Example

```c++
/**Creates cache with maximum size of three. When the 
   size in achieved every next element will replace the 
   least recently used one */
guiorgy::lru_cache<std::string, std::string, 3> cache;

/**Preallocates the necessary memory to avoid reallocations.
   Alternatively you can use guiorgy::lru_cache_opts with
   guiorgy::LruCacheOptions::Preallocate, which will execute
   reserve() during initialization */
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

The underlying hashmap implementation can be changed from the default `std::unordered_map` to any other that has the following members similar to `std::unordered_map`:

- todo

To switch to the desired implementation, specify it as a template argument:

    ```c++
    guiorgy::lru_cache<std::string, std::string, 3, absl::flat_hash_map> cache;
    ```

You can also change the default hash function and key equality predecate:

    ```c++
    guiorgy::lru_cache<std::string, std::string, 3, absl::flat_hash_map, std::hash<std::string>, std::equal_to<std::string>> cache;
    ```

The following hashmap implementation versions are tested by executing full-test.sh:

- [STD](https://en.cppreference.com/w/cpp/container/unordered_map) `std::unordered_map`
- [Abseil](https://github.com/abseil/abseil-cpp) `absl::flat_hash_map` (20250512.0)
- [Tessil Sparse](https://github.com/Tessil/sparse-map) `tsl::sparse_map` (v0.6.2)
- [Tessil Robin](https://github.com/Tessil/sparse-map) `tsl::robin_map` (v1.4.0)
- [Tessil Hopscotch](https://github.com/Tessil/sparse-map) `tsl::hopscotch_map` (v2.3.1)
- [Ankerl](https://github.com/martinus/unordered_dense) `ankerl::unordered_dense::map` (v4.5.0)
- [Ankerl Segmented](https://github.com/martinus/unordered_dense) `ankerl::unordered_dense::segmented_map` (v4.5.0)

To decide which implementation to use, check out the amazing round of [benchmarks](https://martin.ankerl.com/2022/08/27/hashmap-bench-01/) for many map implementations done by Martin Leitner-Ankerl.

### How to run tests

```shell
cd build
cmake ..
# cmake -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=Debug -DCPPCHECK=ON -DHASH_MAP_IMPLEMENTATION=STD|ABSEIL|TESSIL_SPARSE|TESSIL_ROBIN|TESSIL_HOP|ANKERL|ANKERL_SEG ..
make test
# make sanitize
# ./full-test.sh
# docker compose run --rm fulltest
```

### TODO

- Make it possible to pass a custom map container as a template parameter (as long as it has std::unordered_map-like API that we need)
- Use std::array internally instead of std::vector when the Preallocate option is set
- Optionally allow the use of std::deque internally instead of std::vector, when pointer/reference stability for value is needed and/or when copy/move is too expensive during vector growth
- Optionally use a set instead of a map, when pointer/reference stability for key is needed and/or when copy/move is too expensive
- Optionally disable element removal to save a bit of space on tracking "empty" nodes that can be rewritten
- Readd tests for std::pmr::unordered_map
- Add tests for [gtl::parallel_flat_hash_map](https://github.com/greg7mdp/gtl)
- Add tests for [folly::F14ValueMap](https://github.com/facebook/folly)
- Add tests for [emhash8::HashMap](https://github.com/ktprime/emhash)
- More tests
- Fuzzing
- Benchmarks
