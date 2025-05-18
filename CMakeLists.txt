cmake_minimum_required(VERSION 3.12...3.29)

project(CPP-LRU-CACHE-TEST LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Werror")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g3 -fno-omit-frame-pointer")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -g -flto=auto -DNDEBUG")
add_compile_definitions(LRU_CACHE_PRINT_HASH_MAP_IMPLEMENTATION)
set(CMAKE_LINK_FLAGS_RELEASE "${CMAKE_LINK_FLAGS_RELEASE} -flto=auto")

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

option(ABSEIL "Use absl::flat_hash_map instead of stl::unordered_map" OFF)
if (ABSEIL)
    message(STATUS "Using absl::flat_hash_map instead of stl::unordered_map")

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

option(TESSIL "Use tsl::sparse_map instead of stl::unordered_map" OFF)
if (TESSIL)
    message(STATUS "Using tsl::sparse_map instead of stl::unordered_map")

    add_compile_definitions(LRU_CACHE_HASH_MAP_IMPLEMENTATION=TESSIL_SPARSE_MAP)

    include(ExternalProject)
    ExternalProject_Add(
        tessil
        GIT_REPOSITORY https://github.com/Tessil/sparse-map.git
        GIT_TAG v0.6.2
        SOURCE_DIR "${CMAKE_BINARY_DIR}/tessil-src"
        BINARY_DIR "${CMAKE_BINARY_DIR}/tessil-bin"
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
        UPDATE_COMMAND ""
    )

    set(HASH_MAP_IMPLEMENTATION_INCLUDE_DIRECTORIES "${CMAKE_BINARY_DIR}/tessil-src/include")
endif()

include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${HASH_MAP_IMPLEMENTATION_INCLUDE_DIRECTORIES}
)

file(GLOB TESTS ${PROJECT_SOURCE_DIR}/tests/*.cpp)
add_executable(cpp-lru-cache-test ${TESTS})

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
    if(CMAKE_C_COMPILER_ID MATCHES "GNU")
        add_compile_options(-static-libasan -static-libmsan -static-libubsan)
        add_link_options(-static-libasan -static-libmsan -static-libubsan)
    endif()

    add_custom_target(
        sanitize
        COMMAND ${VALGRIND_EXECUTABLE} --verbose --tool=memcheck --leak-check=full --leak-resolution=high --show-leak-kinds=all --track-origins=yes --show-error-list=yes --error-exitcode=1 "$<TARGET_FILE:cpp-lru-cache-test>" --verbose --gtest_color=yes
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        DEPENDS cpp-lru-cache-test
        USES_TERMINAL
        VERBATIM
    )
else()
    message(WARNING "Valgrind not found. Disabling the 'sanitize' target")
endif()
