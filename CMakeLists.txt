cmake_minimum_required(VERSION 3.29)

project(CPP-LRU-CACHE-TEST LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Werror")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g3 -fsanitize=address -fsanitize=leak -DLD_PRELOAD")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -g -flto=auto -DNDEBUG")
set(CMAKE_LINK_FLAGS_RELEASE "${CMAKE_LINK_FLAGS_RELEASE} -flto=auto")

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
    CMAKE_ARGS -Dgtest_force_shared_crt=ON
)
FetchContent_MakeAvailable(googletest)

enable_testing()
include(CTest)
list(APPEND CMAKE_CTEST_ARGUMENTS "--verbose")
set(CMAKE_SKIP_TEST_ALL_DEPENDENCY FALSE)

include_directories(${PROJECT_SOURCE_DIR}/include)

file(GLOB TESTS ${PROJECT_SOURCE_DIR}/tests/*.cpp)
add_executable(cpp-lru-cache-test ${TESTS})

target_link_libraries(cpp-lru-cache-test gtest gtest_main)

include(GoogleTest)
add_test(
    NAME cpp-lru-cache-test
    COMMAND cpp-lru-cache-test --verbose --gtest_color=yes
)

find_program(VALGRIND_EXECUTABLE valgrind)
if(VALGRIND_EXECUTABLE)
    add_custom_target(
        valgrind
        COMMAND ${VALGRIND_EXECUTABLE} --verbose --tool=memcheck --leak-check=full --leak-resolution=high --show-leak-kinds=all --track-origins=yes --show-error-list=yes --error-exitcode=1 "$<TARGET_FILE:cpp-lru-cache-test>" --verbose --gtest_color=yes
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        DEPENDS cpp-lru-cache-test
        USES_TERMINAL
        VERBATIM
    )
else()
    message(WARNING "Valgrind not found. Disabling the 'valgrind' target")
endif()
