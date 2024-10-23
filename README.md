# cpp-lru-cache

> ⚠️ **This implementation is NOT thread safe!**

Simple and reliable LRU (Least Recently Used) cache for c++ based on hashmap and linkedlist. The library is header only, simple test and example are included.
It includes standard components and very little own logics that guarantees reliability.

Example:

```c++
/**Creates cache with maximum size of three. When the 
   size in achieved every next element will replace the 
   least recently used one */
lru_cache<std::string, std::string, 3> cache;

cache.put("one", "one");
cache.put("two", "two");

const std::string& from_cache = cache.get("two").value();

```

How to run tests:

```shell
mkdir build
cd build
cmake ..
make check
```
