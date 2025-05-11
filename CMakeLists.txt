cmake_minimum_required(VERSION 3.5)

project(CPP-LRU-CACHE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Werror")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g3")
set(CMAKE_CSS_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -03 -g -DNDEBUG")

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
    CMAKE_ARGS -Dgtest_force_shared_crt=ON
)
FetchContent_MakeAvailable(googletest)
enable_testing()

include_directories(${PROJECT_SOURCE_DIR}/include)

file(GLOB TESTS ${PROJECT_SOURCE_DIR}/tests/*.cpp)
add_executable(cpp-lru-cache-test ${TESTS})

target_link_libraries(cpp-lru-cache-test gtest gtest_main)

include(GoogleTest)
add_test(
    NAME cpp-lru-cache-test
    COMMAND cpp-lru-cache-test
)

add_custom_target(
    check
    COMMAND ${CMAKE_CTEST_COMMAND} --verbose
    DEPENDS cpp-lru-cache-test
    VERBATIM
)
