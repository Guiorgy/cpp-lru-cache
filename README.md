# cpp-lru-cache

> ⚠️ **This implementation is NOT thread safe!**

Simple and reliable LRU (Least Recently Used) cache for c++ 17 based on hashmap and linkedlist. The library is header only, simple test and example are included.
It includes standard components and very little own logics that guarantees reliability.

Example:

```c++
/**Creates cache with maximum size of three. When the 
   size is achieved every next element will replace the 
   least recently used one */
lru_cache<int, std::string, 3> cache;

/**All three insertion methods (copy, move and emplace)
   of the value are supported */
const std::string one = "one";
cache.put(1, one);
std::string two = "two";
cache.put(2, std::move(two));
const char* three_arg = "three";
const std::string& three = cache.emplace(3, three_arg, std::allocator<char>{});

/**Returns a std::optional with a copy of the value
   (if the key exists), and pushes the key to the
   back of the LRU queue */
std::string from_cache_1 = cache.get(1).value();

/**For objects too expensive or impossible to copy
   you can get them by reference, however, no guarantees
   are given about the underlying object lifetime
   when modifying the cache, so use with caution */
const std::string& from_cache_2_ref = cache.get_ref(2)->get();

/**If you don't like using std::optional, old style
   try_get and try_get_ref are also available */
std::string from_cache_3{};
bool success = cache.try_get(3, from_cache_3);
const std::string* from_cache_3_ptr = nullptr;
success = cache.try_get_ref(3, from_cache_3_ptr);

/**Pushes the given key to the back of the LRU queue. */
cache.touch(1);
cache.put(4, "four"); // max_size is 3 so LRU key is removed
if (cache.exists(2)) std::unreachable();

/**Removes the given key from cache */
cache.remove(4);
if (cache.exists(4)) std::unreachable();

/**You can iterate through the cache, from most
   recently used to least recently used. This does
   not change the LRU queue order */
for (auto it = cache.begin(); it != cache.end(); ++it) {
   int key = it->first;
   const std::string& value = it->second;
}
```

How to run tests:

```shell
cd build
cmake ..
# cmake -DCMAKE_BUILD_TYPE=Debug -DCPPCHECK=ON ..
make test
# make sanitize
```
