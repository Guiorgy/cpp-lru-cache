cmake_minimum_required(VERSION 3.14...3.29)

project(CPP-LRU-CACHE-TEST LANGUAGES CXX)

set(CMAKE_ALLOWED_BUILD_TYPES "Debug" "Release") # "MinSizeRel" "RelWithDebInfo"
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
elseif(NOT "${CMAKE_BUILD_TYPE}" IN_LIST CMAKE_ALLOWED_BUILD_TYPES)
    message(FATAL_ERROR "Invalid CMAKE_BUILD_TYPE: '${CMAKE_BUILD_TYPE}'. Allowed types are: ${CMAKE_ALLOWED_BUILD_TYPES}")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Werror -fmax-errors=10")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g3 -fno-omit-frame-pointer")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -g -DNDEBUG")

set(MORE_WARNING_FLAGS "")
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 11.0.0)
        set(MORE_WARNING_FLAGS "${MORE_WARNING_FLAGS} -Walloc-zero -Walloca -Warray-bounds=2 -Wattribute-alias=2 -Wcast-align=strict -Wcast-qual -Wcatch-value=3 -Wcomma-subscript -Wconditionally-supported -Wconversion -Wctad-maybe-unsupported -Wctor-dtor-privacy -Wdate-time -Wdeprecated -Wdisabled-optimization -Wdouble-promotion -Wduplicated-branches -Wduplicated-cond -Weffc++ -Wextra-semi -Wfloat-equal -Wformat=2 -Wimplicit-fallthrough=5 -Winvalid-imported-macros -Wlogical-op -Wlong-long -Wmisleading-indentation -Wmismatched-tags -Wmissing-declarations -Wmissing-format-attribute -Wmissing-include-dirs -Wmissing-noreturn -Wmultiple-inheritance -Wnoexcept -Wnull-dereference -Wold-style-cast -Woverloaded-virtual -Wpacked -Wplacement-new=2 -Wredundant-decls -Wredundant-tags -Wshadow -Wshift-overflow=2 -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=5 -Wstringop-overflow=4 -Wsuggest-attribute=noreturn -Wsuggest-final-methods -Wsuggest-final-types -Wsuggest-override -Wswitch-default -Wswitch-enum -Wtautological-compare -Wtrampolines -Wundef -Wunsafe-loop-optimizations -Wunused-const-variable=2 -Wunused-macros -Wuseless-cast -Wvector-operation-performance -Wvirtual-inheritance -Wzero-as-null-pointer-constant -Wzero-length-bounds")
    else()
        message(WARNING "GNU compiler version too low and untested. Additional warnings disabled")
    endif()

    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 12.0.0)
        set(MORE_WARNING_FLAGS "${MORE_WARNING_FLAGS} -Wbidi-chars=any,ucn -Winterference-size -Wtrivial-auto-var-init -Wuse-after-free=3")
    endif()

    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 13.0.0)
        set(MORE_WARNING_FLAGS "${MORE_WARNING_FLAGS} -Winvalid-utf8")
    endif()

    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 14.0.0)
        set(MORE_WARNING_FLAGS "${MORE_WARNING_FLAGS} -Wnrvo")
    endif()

    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 15.0.0)
        set(MORE_WARNING_FLAGS "${MORE_WARNING_FLAGS} -Wleading-whitespace=tabs -Wtrailing-whitespace=any")
    endif()

    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 16.0.0)
        message(WARNING "GNU compiler version higher than tested")
    endif()

    # Excluded warnings: -Waggregate-return -Wflex-array-member-not-at-end -Winline -Winvalid-pch -Wnamespaces -Wnon-virtual-dtor -Wnormalized=nfkc -Wopenacc-parallelism -Wopenmp-simd -Wpadded -Wstack-protector -Wstrict-flex-arrays -Wsuggest-attribute=cold -Wsuggest-attribute=const -Wsuggest-attribute=format -Wsuggest-attribute=malloc -Wsuggest-attribute=pure -Wsuggest-attribute=returns_nonnull -Wsystem-headers -Wtemplates
else()
    message(WARNING "Untested compiler '${CMAKE_CXX_COMPILER_ID}'. Additional warnings disabled")
endif()

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
    CMAKE_ARGS -Dgtest_force_shared_crt=ON
)
set(FETCH_CONTENT_MAKE_AVAILABLE googletest)

option(HASH_MAP_IMPLEMENTATION "Select the implementation of hashmap to use inside the LRU cache. std::unordered_map by default" "")

set(HASH_MAP_IMPLEMENTATION_INCLUDE_DIRECTORIES "")
set(HASH_MAP_IMPLEMENTATION_LINK_LIBRARIES "")

if (HASH_MAP_IMPLEMENTATION STREQUAL "")
    message(STATUS "Using the default hashmap implementation")
elseif (HASH_MAP_IMPLEMENTATION STREQUAL "STL")
    message(STATUS "Using std::unordered_map as the hashmap implementation")

    add_compile_definitions(LRU_CACHE_HASH_MAP_IMPLEMENTATION=STL_UNORDERED_MAP)
elseif (HASH_MAP_IMPLEMENTATION STREQUAL "STL_PMR")
    message(STATUS "Using std::pmr::unordered_map with std::pmr::unsynchronized_pool_resource as the hashmap implementation")

    add_compile_definitions(LRU_CACHE_HASH_MAP_IMPLEMENTATION=STL_PMR_UNORDERED_MAP)
elseif (HASH_MAP_IMPLEMENTATION STREQUAL "ABSEIL")
    message(STATUS "Using absl::flat_hash_map as the hashmap implementation")

    add_compile_definitions(LRU_CACHE_HASH_MAP_IMPLEMENTATION=ABSEIL_FLAT_HASH_MAP)

    FetchContent_Declare(
        abseil
        GIT_REPOSITORY https://github.com/abseil/abseil-cpp.git
        GIT_TAG 20250512.0
        CMAKE_ARGS -DCMAKE_CXX_STANDARD=17 -DABSL_ENABLE_INSTALL=OFF
        SYSTEM
    )
    list(APPEND FETCH_CONTENT_MAKE_AVAILABLE abseil)

    set(HASH_MAP_IMPLEMENTATION_LINK_LIBRARIES absl::flat_hash_map)
elseif (HASH_MAP_IMPLEMENTATION STREQUAL "TESSIL_SPARSE")
    message(STATUS "Using tsl::sparse_map as the hashmap implementation")

    add_compile_definitions(LRU_CACHE_HASH_MAP_IMPLEMENTATION=TESSIL_SPARSE_MAP)

    FetchContent_Declare(
        tessil-sparse
        GIT_REPOSITORY https://github.com/Tessil/sparse-map.git
        GIT_TAG v0.6.2
        SOURCE_SUBDIR "include"
        SYSTEM
    )
    if (CMAKE_VERSION VERSION_GREATER_EQUAL 3.30)
        list(APPEND FETCH_CONTENT_MAKE_AVAILABLE tessil-sparse)
    else()
        FetchContent_Populate(tessil-sparse)
    endif()

    list(APPEND HASH_MAP_IMPLEMENTATION_INCLUDE_DIRECTORIES "tessil-sparse,include")
elseif (HASH_MAP_IMPLEMENTATION STREQUAL "TESSIL_ROBIN")
    message(STATUS "Using tsl::robin_map as the hashmap implementation")

    add_compile_definitions(LRU_CACHE_HASH_MAP_IMPLEMENTATION=TESSIL_ROBIN_MAP)

    FetchContent_Declare(
        tessil-robin
        GIT_REPOSITORY https://github.com/Tessil/robin-map.git
        GIT_TAG v1.4.0
        SOURCE_SUBDIR "include"
        SYSTEM
    )
    if (CMAKE_VERSION VERSION_GREATER_EQUAL 3.30)
        list(APPEND FETCH_CONTENT_MAKE_AVAILABLE tessil-robin)
    else()
        FetchContent_Populate(tessil-robin)
    endif()

    list(APPEND HASH_MAP_IMPLEMENTATION_INCLUDE_DIRECTORIES "tessil-robin,include")
elseif (HASH_MAP_IMPLEMENTATION STREQUAL "TESSIL_HOP")
    message(STATUS "Using tsl::hopscotch_map as the hashmap implementation")

    add_compile_definitions(LRU_CACHE_HASH_MAP_IMPLEMENTATION=TESSIL_HOPSCOTCH_MAP)

    FetchContent_Declare(
        tessil-hopscotch
        GIT_REPOSITORY https://github.com/Tessil/hopscotch-map.git
        GIT_TAG v2.3.1
        SOURCE_SUBDIR "include"
        SYSTEM
    )
    if (CMAKE_VERSION VERSION_GREATER_EQUAL 3.30)
        list(APPEND FETCH_CONTENT_MAKE_AVAILABLE tessil-hopscotch)
    else()
        FetchContent_Populate(tessil-hopscotch)
    endif()

    list(APPEND HASH_MAP_IMPLEMENTATION_INCLUDE_DIRECTORIES "tessil-hopscotch,include")
elseif (HASH_MAP_IMPLEMENTATION STREQUAL "ANKERL" OR HASH_MAP_IMPLEMENTATION STREQUAL "ANKERL_SEG")
    if (HASH_MAP_IMPLEMENTATION STREQUAL "ANKERL")
        message(STATUS "Using ankerl::unordered_dense::map as the hashmap implementation")

        add_compile_definitions(LRU_CACHE_HASH_MAP_IMPLEMENTATION=ANKERL_UNORDERED_DENSE_MAP)
    else()
        message(STATUS "Using ankerl::unordered_dense::segmented_map as the hashmap implementation")

        add_compile_definitions(LRU_CACHE_HASH_MAP_IMPLEMENTATION=ANKERL_UNORDERED_DENSE_SEGMENTED_MAP)
    endif()

    FetchContent_Declare(
        ankerl
        GIT_REPOSITORY https://github.com/martinus/unordered_dense.git
        GIT_TAG v4.5.0
        SYSTEM
    )
    list(APPEND FETCH_CONTENT_MAKE_AVAILABLE ankerl)

    set(HASH_MAP_IMPLEMENTATION_LINK_LIBRARIES unordered_dense)
else()
    message(FATAL_ERROR "Invalid hashmap implementation set in HASH_MAP_IMPLEMENTATION: ${HASH_MAP_IMPLEMENTATION}")
endif()

FetchContent_MakeAvailable(${FETCH_CONTENT_MAKE_AVAILABLE})

set(TMP "")
foreach(name_and_include IN LISTS HASH_MAP_IMPLEMENTATION_INCLUDE_DIRECTORIES)
    string(REPLACE "," ";" name_and_include "${name_and_include}")

    list(GET name_and_include 0 content_name)
    list(GET name_and_include 1 include_directory)

    if (NOT content_name STREQUAL "")
        list(APPEND TMP "${${content_name}_SOURCE_DIR}/${include_directory}")
    else()
        list(APPEND TMP "${include_directory}")
    endif()
endforeach()
set(HASH_MAP_IMPLEMENTATION_INCLUDE_DIRECTORIES ${TMP})

file(GLOB_RECURSE TESTS ${PROJECT_SOURCE_DIR}/tests/*.cpp)
add_executable(cpp-lru-cache-test ${TESTS})

target_compile_options(cpp-lru-cache-test PRIVATE "SHELL:${MORE_WARNING_FLAGS}")

target_include_directories(
    cpp-lru-cache-test SYSTEM PRIVATE
    ${HASH_MAP_IMPLEMENTATION_INCLUDE_DIRECTORIES}
)
target_include_directories(
    cpp-lru-cache-test PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/tests/include
)
target_link_libraries(
    cpp-lru-cache-test PRIVATE
    ${HASH_MAP_IMPLEMENTATION_LINK_LIBRARIES}
    gtest gtest_main
)

include(GoogleTest)
add_test(
    NAME cpp-lru-cache-test
    COMMAND cpp-lru-cache-test --verbose --gtest_color=yes
)

enable_testing()
include(CTest)
list(APPEND CMAKE_CTEST_ARGUMENTS "--verbose")

if (CMAKE_VERSION VERSION_GREATER_EQUAL 3.29)
    set(CMAKE_SKIP_TEST_ALL_DEPENDENCY FALSE)
else()
    message(WARNING "CMake version < 3.29, adding a bogus test that executes 'cmake --build .' as the first test")

    add_test(
        NAME cmake-build
        COMMAND cmake --build .
    )
endif()

find_program(VALGRIND_EXECUTABLE valgrind)
if(VALGRIND_EXECUTABLE)
    add_executable(cpp-lru-cache-test-sanitize ${TESTS})
    target_compile_options(cpp-lru-cache-test-sanitize PRIVATE "SHELL:${MORE_WARNING_FLAGS}")
    target_compile_definitions(cpp-lru-cache-test-sanitize PRIVATE SKIP_ALLOCATION_TESTS)

    target_include_directories(
        cpp-lru-cache-test-sanitize SYSTEM PRIVATE
        ${HASH_MAP_IMPLEMENTATION_INCLUDE_DIRECTORIES}
    )
    target_include_directories(
        cpp-lru-cache-test-sanitize PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/tests/include
    )
    target_link_libraries(
        cpp-lru-cache-test-sanitize PRIVATE
        ${HASH_MAP_IMPLEMENTATION_LINK_LIBRARIES}
        gtest gtest_main
    )

    add_compile_options(
        -fsanitize=address -fsanitize-address-use-after-scope
        -fsanitize=leak
        -fsanitize=memory -fsanitize-memory-track-origins -fno-optimize-sibling-calls
        -fsanitize=undefined -fno-sanitize-recover=all
    )
    add_link_options(
        -fsanitize=address -fsanitize-address-use-after-scope
        -fsanitize=leak
        -fsanitize=memory -fsanitize-memory-track-origins -fno-optimize-sibling-calls
        -fsanitize=undefined -fno-sanitize-recover=all
    )
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        add_compile_options(-static-libasan -static-libmsan -static-libubsan)
        add_link_options(-static-libasan -static-libmsan -static-libubsan)
    endif()

    add_custom_target(
        sanitize
        COMMAND ${VALGRIND_EXECUTABLE} --verbose --tool=memcheck --leak-check=full --leak-resolution=high --show-leak-kinds=all --track-origins=yes --show-error-list=yes --error-exitcode=1 "$<TARGET_FILE:cpp-lru-cache-test-sanitize>" --verbose --gtest_color=yes
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        DEPENDS cpp-lru-cache-test-sanitize
        USES_TERMINAL
        VERBATIM
    )
else()
    message(WARNING "Valgrind not found. Disabling the 'sanitize' target")
endif()
