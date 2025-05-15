# cpp-lru-cache

> ⚠️ **This implementation is NOT thread safe!**

An LRU (Least Recently Used) cache for c++ based on hashmap and a packed linkedlist based on vector. The library is header only, a simple test and example are included.

> ⚠️ *For a simpler implementation that only includes standard components and very little of its own logics (and probably more reliable) see the stl-only branch.*

Example:

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

How to run tests:

```shell
cd build
cmake ..
make check
```
