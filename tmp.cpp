#include <unordered_map>
#include <unordered_set>
#include <type_traits>
#include <iostream>


//template<typename, typename = void


template<typename key_type>
class DefaultHashFunction final {};
template<typename key_type>
class DefaultKeyEqualityPredicate final {};
class NoValueType final {};


template<
    template<typename...> class map_template,
    typename key_type,
    typename value_type,
    typename hash_function = DefaultHashFunction<key_type>,
    typename key_equality_predicate = DefaultKeyEqualityPredicate<key_type>,
    typename... other_args
>
struct HashMapOrVoid final {
    static_assert(
        std::is_same_v<key_equality_predicate, DefaultKeyEqualityPredicate<key_type>>
        || !std::is_same_v<hash_function, DefaultHashFunction<key_type>>,
        "hash_function can't be default if key_equality_predicate is not default"
    );
    static_assert(
        !std::is_same_v<key_equality_predicate, DefaultKeyEqualityPredicate<key_type>>
        || sizeof...(other_args) == 0u,
        "other_args can't be defined if key_equality_predicate is default"
    );

    using type = typename std::conditional_t<
        std::is_same_v<hash_function, DefaultHashFunction<key_type>>,
        map_template<key_type, value_type>,
        typename std::conditional_t<
            std::is_same_v<key_equality_predicate, DefaultKeyEqualityPredicate<key_type>>,
            map_template<key_type, value_type, hash_function>,
            map_template<key_type, value_type, hash_function, key_equality_predicate, other_args...>
        >
    >;
};

template<
    template<typename...> class map_template,
    typename key_type,
    typename hash_function,
    typename key_equality_predicate,
    typename... other_args
>
struct HashMapOrVoid<map_template, key_type, NoValueType, hash_function, key_equality_predicate, other_args...> {
    using type = void;
};

template<
    template<typename...> class set_template,
    typename key_type,
    typename value_type,
    typename hash_function = DefaultHashFunction<key_type>,
    typename key_equality_predicate = DefaultKeyEqualityPredicate<key_type>,
    typename... other_args
>
struct HashSetOrVoid final {
    using type = void;
};

template<
    template<typename...> class set_template,
    typename key_type,
    typename hash_function,
    typename key_equality_predicate,
    typename... other_args
>
struct HashSetOrVoid<set_template, key_type, NoValueType, hash_function, key_equality_predicate, other_args...> {
    static_assert(
        std::is_same_v<key_equality_predicate, DefaultKeyEqualityPredicate<key_type>>
        || !std::is_same_v<hash_function, DefaultHashFunction<key_type>>,
        "hash_function can't be default if key_equality_predicate is not default"
    );
    static_assert(
        !std::is_same_v<key_equality_predicate, DefaultKeyEqualityPredicate<key_type>>
        || sizeof...(other_args) == 0u,
        "other_args can't be defined if key_equality_predicate is default"
    );

    using type = typename std::conditional_t<
        std::is_same_v<hash_function, DefaultHashFunction<key_type>>,
        set_template<key_type>,
        typename std::conditional_t<
            std::is_same_v<key_equality_predicate, DefaultKeyEqualityPredicate<key_type>>,
            set_template<key_type, hash_function>,
            set_template<key_type, hash_function, key_equality_predicate, other_args...>
        >
    >;
};



/*template<
    template<typename...> class map_template,
    typename key_type,
    typename value_type,
    typename hash_function = DefaultHashFunction<key_type>,
    typename key_equality_predicate = DefaultKeyEqualityPredicate<key_type>,
    typename = void,
    typename... other_args
>
struct is_hash_map_like final : std::false_type {};
template<
    template<typename...> class map_template,
    typename key_type,
    typename value_type,
    typename hash_function,
    typename key_equality_predicate,
    typename... other_args
>
struct is_hash_map_like<
    map_template,
    key_type,
    value_type,
    hash_function,
    key_equality_predicate,
    std::void_t<
        decltype(map_template<key_type, value_type, hash_function, key_equality_predicate, other_args...>())
    >,
    other_args...
> final : std::true_type {};*/

template <typename, typename = void>
struct is_map_like : std::false_type {};

template <typename T>
struct is_map_like<T, std::void_t<
    typename T::key_type,
    typename T::mapped_type,
    decltype(std::declval<T&>().size()),
    decltype(std::declval<T&>().find(std::declval<const typename T::key_type&>())),
    decltype(std::declval<T&>().insert(std::declval<std::pair<typename T::key_type, typename T::mapped_type>>()))
>> : std::true_type {};

template <typename, typename = void>
struct is_set_like : std::false_type {};

template <typename T>
struct is_set_like<T, std::void_t<
    typename T::value_type,
    decltype(std::declval<T&>().size()),
    decltype(std::declval<T&>().find(std::declval<const typename T::value_type&>())),
    decltype(std::declval<T&>().insert(std::declval<typename T::value_type>()))
>> : std::true_type {};




template<
    template<typename...> class container_template,
    typename key_type,
    typename value_type,
    typename hash_function,
    typename key_equality_predicate,
    typename = void,
    typename... other_args
>
class HashMapSetWrapper {
    static_assert(false, "base");
};

template<
    template<typename...> class map_template,
    typename key_type,
    typename value_type,
    typename hash_function,
    typename key_equality_predicate,
    typename... other_args
>
class HashMapSetWrapper<map_template, key_type, value_type, hash_function, key_equality_predicate,
    std::enable_if_t<is_map_like<map_template<key_type, value_type, hash_function, key_equality_predicate, other_args...>>::value>,
    other_args...
> {
public:
    using hashmap_type = map_template<key_type, value_type, hash_function, key_equality_predicate, other_args...>;
    using hashset_type = void;
    static constexpr bool is_hash_map = true;
};

template<
    template<typename...> class set_template,
    typename key_type,
    typename value_type,
    typename hash_function,
    typename key_equality_predicate,
    typename... other_args
>
class HashMapSetWrapper<set_template, key_type, value_type, hash_function, key_equality_predicate,
    std::enable_if_t<is_set_like<set_template<value_type, hash_function, key_equality_predicate, other_args...>>::value>,
    other_args...
> {
public:
    using hashmap_type = void;
    using hashset_type = set_template<value_type, hash_function, key_equality_predicate, other_args...>;
    static constexpr bool is_hash_map = false;
};


/*template<
    template<typename, typename, typename, typename, typename...> class hashmap_or_set_template,
    typename key_type,
    typename value_type,
    typename hash_function = DefaultHashFunction<key_type>,
    typename key_equality_predicate = DefaultKeyEqualityPredicate<key_type>,
    typename... other_args
>
class HashMapSetWrapper {
    static_assert(
        std::is_same_v<key_equality_predicate, DefaultKeyEqualityPredicate<key_type>>
        || !std::is_same_v<hash_function, DefaultHashFunction<key_type>>,
        "hash_function can't be default if key_equality_predicate is not default"
    );
    static_assert(
        !std::is_same_v<key_equality_predicate, DefaultKeyEqualityPredicate<key_type>>
        || sizeof...(other_args) == 0u,
        "other_args can't be defined if key_equality_predicate is default"
    );

    template<
        template<typename...> class map_template,
        typename Key,
        typename Value,
        typename Hash = DefaultHashFunction<Key>,
        typename KeyEqual = DefaultKeyEqualityPredicate<Key>,
        typename... OtherArgs
    >
    struct HashMapOrVoid final {
        using type = typename std::conditional_t<
            std::is_same_v<Hash, DefaultHashFunction<Key>>,
            map_template<Key, Value>,
            typename std::conditional_t<
                std::is_same_v<KeyEqual, DefaultKeyEqualityPredicate<Key>>,
                map_template<Key, Value, Hash>,
                map_template<Key, Value, Hash, KeyEqual, OtherArgs...>
            >
        >;
    };

    template<
        template<typename...> class map_template,
        typename Key,
        typename Hash,
        typename KeyEqual,
        typename... OtherArgs
    >
    struct HashMapOrVoid<map_template, Key, NoValueType, Hash, KeyEqual, OtherArgs...> {
        using type = void;
    };

public:
    using hashmap_type = typename HashMapOrVoid<hashmap_or_set_template, key_type, value_type, hash_function, key_equality_predicate, other_args...>::type;
    using hashset_type = typename HashSetOrVoid<hashmap_or_set_template, key_type, value_type, hash_function, key_equality_predicate, other_args...>::type;
    static constexpr bool is_hash_map = is_hash_map_like<hashmap_or_set_template, key_type, value_type, hash_function, key_equality_predicate, other_args...>::value;
};
*/
template<
    template<typename...> class set_template,
    typename key_type,
    typename hash_function = DefaultHashFunction<key_type>,
    typename key_equality_predicate = DefaultKeyEqualityPredicate<key_type>,
    typename... other_args
>
using HashSetWrapper = HashMapSetWrapper<set_template, NoValueType, key_type, hash_function, key_equality_predicate, other_args...>;


int wrap_map() {
    using wrapper_t = HashMapSetWrapper<std::unordered_map, int, int, std::hash<int>, std::equal_to<int>>;
    //wrapper_t map;
    
    std::cout << "map wrapper:\n"
        << "  hashmap_type: " << typeid(wrapper_t::hashmap_type).name() << std::endl
        << "  hashset_type: " << typeid(wrapper_t::hashset_type).name() << std::endl
        << "  is_hash_map: " << wrapper_t::is_hash_map << std::endl;

    return 0;
}

int wrap_set() {
    using wrapper_t = HashSetWrapper<std::unordered_set, int, std::hash<int>, std::equal_to<int>>;
    //wrapper_t set;
    
    std::cout << "set wrapper:\n"
        << "  hashmap_type: " << typeid(wrapper_t::hashmap_type).name() << std::endl
        << "  hashset_type: " << typeid(wrapper_t::hashset_type).name() << std::endl
        ;//<< "  is_hash_map: " << wrapper_t::is_hash_map << std::endl;

    return 0;
}

int main() {
    int x = 0;
    
    x += wrap_map();
    x += wrap_set();

    return x;
}
