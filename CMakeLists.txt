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

if (EXISTS "${CMAKE_BINARY_DIR}/lib/libgtest.a" AND EXISTS "${CMAKE_BINARY_DIR}/lib/libgtest_main.a")
    set(GTEST_INCLUDE_DIR "${CMAKE_BINARY_DIR}/_deps/googletest-src/googletest/include/")

    add_library(gtest STATIC IMPORTED GLOBAL)
    set_target_properties(
        gtest PROPERTIES
        IMPORTED_LOCATION "${CMAKE_BINARY_DIR}/lib/libgtest.a"
        INTERFACE_INCLUDE_DIRECTORIES "${GTEST_INCLUDE_DIR}"
        INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${GTEST_INCLUDE_DIR}"
    )

    add_library(gtest_main STATIC IMPORTED GLOBAL)
    set_target_properties(
        gtest_main PROPERTIES
        IMPORTED_LOCATION "${CMAKE_BINARY_DIR}/lib/libgtest_main.a"
        INTERFACE_INCLUDE_DIRECTORIES "${GTEST_INCLUDE_DIR}"
        INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${GTEST_INCLUDE_DIR}"
    )
else()
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.17.0
        GIT_SHALLOW 1
        SYSTEM
    )
    set(gtest_force_shared_crt ON)
    FetchContent_MakeAvailable(googletest)
endif()

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

file(GLOB TESTS ${PROJECT_SOURCE_DIR}/tests/*.cpp)
add_executable(cpp-lru-cache-test ${TESTS})

target_compile_options(cpp-lru-cache-test PRIVATE "SHELL:${MORE_WARNING_FLAGS}")

target_include_directories(
    cpp-lru-cache-test PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)
target_link_libraries(
    cpp-lru-cache-test PRIVATE
    gtest gtest_main
)

include(GoogleTest)
add_test(
    NAME cpp-lru-cache-test
    COMMAND cpp-lru-cache-test --verbose --gtest_color=yes
)

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
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
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
