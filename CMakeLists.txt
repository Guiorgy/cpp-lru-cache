cmake_minimum_required(VERSION 3.14...3.29)

project(CPP-LRU-CACHE-TEST LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Werror")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g3 -fno-omit-frame-pointer")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -g -flto=auto -DNDEBUG")
set(CMAKE_LINK_FLAGS_RELEASE "${CMAKE_LINK_FLAGS_RELEASE} -flto=auto")

# Those flags are recognized by GNU G++ 11.4.0
set(MORE_WARNING_FLAGS "${MORE_WARNING_FLAGS} -Walloc-zero -Walloca -Warray-bounds=2 -Wattribute-alias=2 -Wcast-align=strict -Wcast-qual -Wcatch-value=3 -Wcomma-subscript -Wconditionally-supported -Wconversion -Wctad-maybe-unsupported -Wctor-dtor-privacy -Wdate-time -Wdeprecated -Wdisabled-optimization -Wdouble-promotion -Wduplicated-branches -Wduplicated-cond -Weffc++ -Wextra-semi -Wfloat-equal -Wformat=2 -Wimplicit-fallthrough=5 -Winvalid-imported-macros -Wlogical-op -Wlong-long -Wmisleading-indentation -Wmismatched-tags -Wmissing-declarations -Wmissing-format-attribute -Wmissing-include-dirs -Wmissing-noreturn -Wmultiple-inheritance -Wnoexcept -Wnull-dereference -Wold-style-cast -Woverloaded-virtual -Wpacked -Wplacement-new=2 -Wredundant-decls -Wredundant-tags -Wshadow -Wshift-overflow=2 -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=5 -Wstringop-overflow=4 -Wsuggest-attribute=noreturn -Wsuggest-final-methods -Wsuggest-final-types -Wsuggest-override -Wswitch-default -Wswitch-enum -Wtautological-compare -Wtrampolines -Wundef -Wunsafe-loop-optimizations -Wunused-const-variable=2 -Wunused-macros -Wuseless-cast -Wvector-operation-performance -Wvirtual-inheritance -Wzero-as-null-pointer-constant -Wzero-length-bounds")

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
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
endif()

# Excluded warnings: -Waggregate-return -Wflex-array-member-not-at-end -Winline -Winvalid-pch -Wnamespaces -Wnon-virtual-dtor -Wnormalized=nfkc -Wopenacc-parallelism -Wopenmp-simd -Wpadded -Wstack-protector -Wstrict-flex-arrays -Wsuggest-attribute=cold -Wsuggest-attribute=const -Wsuggest-attribute=format -Wsuggest-attribute=malloc -Wsuggest-attribute=pure -Wsuggest-attribute=returns_nonnull -Wsystem-headers -Wtemplates

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
    CMAKE_ARGS -Dgtest_force_shared_crt=ON
)
FetchContent_MakeAvailable(googletest)

set(HASH_MAP_IMPLEMENTATION_INCLUDE_DIRECTORIES "")
set(HASH_MAP_IMPLEMENTATION_LINK_LIBRARIES "")
set(HASH_MAP_IMPLEMENTATION_OPTIONS_ON "")
set(NUM_HASH_MAP_IMPLEMENTATIONS_ON 0)

option(ABSEIL "Use absl::flat_hash_map instead of std::unordered_map" OFF)
if (ABSEIL)
    if (NUM_HASH_MAP_IMPLEMENTATIONS_ON EQUAL 0)
        message(STATUS "Using absl::flat_hash_map instead of std::unordered_map")

        add_compile_definitions(LRU_CACHE_HASH_MAP_IMPLEMENTATION=ABSEIL_FLAT_HASH_MAP)

        FetchContent_Declare(
            abseil
            GIT_REPOSITORY https://github.com/abseil/abseil-cpp.git
            GIT_TAG 20250512.0
            CMAKE_ARGS -DCMAKE_CXX_STANDARD=17 -DABSL_ENABLE_INSTALL=OFF
            SYSTEM
        )
        FetchContent_MakeAvailable(abseil)

        set(HASH_MAP_IMPLEMENTATION_LINK_LIBRARIES absl::flat_hash_map)
    endif()

    list(APPEND HASH_MAP_IMPLEMENTATION_OPTIONS_ON ABSEIL)
    list(LENGTH HASH_MAP_IMPLEMENTATION_OPTIONS_ON NUM_HASH_MAP_IMPLEMENTATIONS_ON)
endif()

option(TESSIL "Use tsl::sparse_map instead of std::unordered_map" OFF)
if (TESSIL)
    if (NUM_HASH_MAP_IMPLEMENTATIONS_ON EQUAL 0)
        message(STATUS "Using tsl::sparse_map instead of std::unordered_map")

        add_compile_definitions(LRU_CACHE_HASH_MAP_IMPLEMENTATION=TESSIL_SPARSE_MAP)

        FetchContent_Declare(
            tessil
            GIT_REPOSITORY https://github.com/Tessil/sparse-map.git
            GIT_TAG v0.6.2
            SYSTEM
        )
        FetchContent_Populate(tessil)

        set(HASH_MAP_IMPLEMENTATION_INCLUDE_DIRECTORIES "${tessil_SOURCE_DIR}/include")
    endif()

    list(APPEND HASH_MAP_IMPLEMENTATION_OPTIONS_ON TESSIL)
    list(LENGTH HASH_MAP_IMPLEMENTATION_OPTIONS_ON NUM_HASH_MAP_IMPLEMENTATIONS_ON)
endif()

option(ANKERL "Use ankerl::unordered_dense::map instead of std::unordered_map" OFF)
option(ANKERL_SEG "Use ankerl::unordered_dense::segmented_map instead of std::unordered_map" OFF)
if (ANKERL OR ANKERL_SEG)
    if (NUM_HASH_MAP_IMPLEMENTATIONS_ON EQUAL 0)
        if (ANKERL)
            message(STATUS "Using ankerl::unordered_dense::map instead of std::unordered_map")

            add_compile_definitions(LRU_CACHE_HASH_MAP_IMPLEMENTATION=ANKERL_UNORDERED_DENSE_MAP)
        else()
            message(STATUS "Using ankerl::unordered_dense::segmented_map instead of std::unordered_map")

            add_compile_definitions(LRU_CACHE_HASH_MAP_IMPLEMENTATION=ANKERL_UNORDERED_DENSE_SEGMENTED_MAP)
        endif()

        FetchContent_Declare(
            ankerl
            GIT_REPOSITORY https://github.com/martinus/unordered_dense.git
            GIT_TAG v4.5.0
            SYSTEM
        )
        FetchContent_MakeAvailable(ankerl)

        set(HASH_MAP_IMPLEMENTATION_LINK_LIBRARIES unordered_dense)
    endif()

    if (ANKERL)
        list(APPEND HASH_MAP_IMPLEMENTATION_OPTIONS_ON ANKERL)
    endif()
    if(ANKERL_SEG)
        list(APPEND HASH_MAP_IMPLEMENTATION_OPTIONS_ON ANKERL_SEG)
    endif()
    list(LENGTH HASH_MAP_IMPLEMENTATION_OPTIONS_ON NUM_HASH_MAP_IMPLEMENTATIONS_ON)
endif()

if(NUM_HASH_MAP_IMPLEMENTATIONS_ON GREATER 1)
    message(FATAL_ERROR "At most only one implementation of the hashmap can be enabled. The following options are mutually exclusive: ${HASH_MAP_IMPLEMENTATION_OPTIONS_ON}")
endif()

include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${HASH_MAP_IMPLEMENTATION_INCLUDE_DIRECTORIES}
    ${PROJECT_SOURCE_DIR}/tests/include
)

file(GLOB_RECURSE TESTS ${PROJECT_SOURCE_DIR}/tests/*.cpp)
add_executable(cpp-lru-cache-test ${TESTS})

target_compile_options(cpp-lru-cache-test PRIVATE "SHELL:${MORE_WARNING_FLAGS}")

target_link_libraries(
    cpp-lru-cache-test
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

    target_link_libraries(
        cpp-lru-cache-test-sanitize
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
