cmake_minimum_required(VERSION 3.14...3.29)

project(CPP-LRU-CACHE-TEST LANGUAGES CXX)

if (CMAKE_VERSION VERSION_LESS 3.21)
    string(COMPARE EQUAL "${PROJECT_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}" PROJECT_IS_TOP_LEVEL)
endif()
if (NOT PROJECT_IS_TOP_LEVEL)
    message(WARNING "lru-cache is a single header only library that doesn't need to be built and linked separately. This CMake project is only used for testing and is not meant to be included in another. Just add the include directory into the list of include directories.")
    return()
endif()

set(CMAKE_ALLOWED_BUILD_TYPES "Debug" "Release") # "MinSizeRel" "RelWithDebInfo"
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
elseif (NOT "${CMAKE_BUILD_TYPE}" IN_LIST CMAKE_ALLOWED_BUILD_TYPES)
    message(FATAL_ERROR "Invalid CMAKE_BUILD_TYPE: '${CMAKE_BUILD_TYPE}'. Allowed types are: ${CMAKE_ALLOWED_BUILD_TYPES}")
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    string(REGEX MATCH "/clang\\+?\\+?-([0-9]+)$" _ "${CMAKE_CXX_COMPILER}")
    set(CLANG_MAJOR_VERSION ${CMAKE_MATCH_1})

    if (CLANG_MAJOR_VERSION)
        set(CMAKE_C_COMPILER /usr/bin/clang-${CLANG_MAJOR_VERSION})
    else()
        set(CMAKE_C_COMPILER /usr/bin/clang)
    endif()

    if (NOT CLANG_MAJOR_VERSION)
        string(REGEX MATCH "^([0-9]+)\\.[0-9]+\\.[0-9]+$" _ "${CMAKE_CXX_COMPILER_VERSION}")
        set(CLANG_MAJOR_VERSION ${CMAKE_MATCH_1})
    endif()

    find_program(LLD_EXECUTABLE NAMES ld.lld-${CLANG_MAJOR_VERSION})
    if (LLD_EXECUTABLE)
        message(STATUS "Uisng ${LLD_EXECUTABLE} with Clang")

        set(CLANG_LINKER_FLAGS "--ld-path=${LLD_EXECUTABLE} -lstdc++ -lm -lc -lgcc_s")
    else()
        message(WARNING "LLVM lld linker not found. Clang will default to system linker")
    endif()

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${CLANG_LINKER_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${CLANG_LINKER_FLAGS}")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${CLANG_LINKER_FLAGS}")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    include(CheckIPOSupported)
    check_ipo_supported(RESULT IPO_SUPPORTED OUTPUT ERROR_OUTPUT LANGUAGES CXX)
    if (IPO_SUPPORTED)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
    else()
        message(WARNING "IPO/LTO is not supported: ${ERROR_OUTPUT}")
    endif()
endif()

set(COMPILER_FLAGS "")
set(COMPILER_FLAGS_DEBUG "")
set(COMPILER_FLAGS_RELEASE "")
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(COMPILER_FLAGS "-Wall -Wextra -Wpedantic -Werror -fmax-errors=10")
    set(COMPILER_FLAGS_DEBUG "-O0 -g3 -fno-omit-frame-pointer -D_GLIBCXX_DEBUG")
    set(COMPILER_FLAGS_RELEASE "-O3 -g -DNDEBUG")

    set(MORE_WARNING_FLAGS "")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 11.0.0)
        set(MORE_WARNING_FLAGS "${MORE_WARNING_FLAGS} -Walloc-zero -Walloca -Warith-conversion -Warray-bounds=2 -Wattribute-alias=2 -Wcast-align=strict -Wcast-qual -Wcatch-value=3 -Wcomma-subscript -Wconditionally-supported -Wconversion -Wctad-maybe-unsupported -Wctor-dtor-privacy -Wdate-time -Wdeprecated -Wdisabled-optimization -Wdouble-promotion -Wduplicated-branches -Wduplicated-cond -Weffc++ -Wenum-conversion -Wextra-semi -Wfloat-equal -Wformat=2 -Wimplicit-fallthrough=5 -Winvalid-imported-macros -Wlogical-op -Wmisleading-indentation -Wmismatched-tags -Wmissing-declarations -Wmissing-format-attribute -Wmissing-include-dirs -Wmissing-noreturn -Wmultiple-inheritance -Wnoexcept -Wnull-dereference -Wold-style-cast -Woverloaded-virtual -Wpacked -Wplacement-new=2 -Wredundant-decls -Wredundant-tags -Wshadow -Wshift-overflow=2 -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=5 -Wstringop-overflow=4 -Wsuggest-attribute=noreturn -Wsuggest-final-methods -Wsuggest-final-types -Wsuggest-override -Wswitch-default -Wswitch-enum -Wtautological-compare -Wtrampolines -Wundef -Wunsafe-loop-optimizations -Wunused-const-variable=2 -Wunused-macros -Wuseless-cast -Wvector-operation-performance -Wvirtual-inheritance -Wzero-as-null-pointer-constant -Wzero-length-bounds")
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

    set(COMPILER_FLAGS "${COMPILER_FLAGS} ${MORE_WARNING_FLAGS}")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(COMPILER_FLAGS "-Weverything -Wpedantic -Werror")
    set(COMPILER_FLAGS_DEBUG "-O0 -g3 -fno-omit-frame-pointer -mno-omit-leaf-frame-pointer -D_GLIBCXX_DEBUG")
    set(COMPILER_FLAGS_RELEASE "-O3 -g -DNDEBUG")

    # Suppress useless warnings.
    set(COMPILER_FLAGS "${COMPILER_FLAGS} -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-missing-prototypes -Wno-padded -Wno-disabled-macro-expansion")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(COMPILER_FLAGS "/Wall /permissive- /WX /external:anglebrackets /external:W0 /EHsc /Zc:__cplusplus")
    set(COMPILER_FLAGS_DEBUG "/Od /Zi /Oy- /D_DEBUG /D_ITERATOR_DEBUG_LEVEL=2")
    set(COMPILER_FLAGS_RELEASE "/O2 /GL /Zi /DNDEBUG /W4") # MSVC with /Wall in Release warns for code inside the included dependencies too

    # Suppress useless warnings.
    set(COMPILER_FLAGS "${COMPILER_FLAGS} /wd4514 /wd4820 /wd5045")
    # C4514: unreferenced inline function has been removed
    # C4820: bytes padding added after construct
    # C5045: Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified
else()
    message(WARNING "Untested compiler '${CMAKE_CXX_COMPILER_ID}'. Additional warnings disabled")
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(COMPILER_FLAGS "${COMPILER_FLAGS} ${COMPILER_FLAGS_DEBUG}")
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(COMPILER_FLAGS "${COMPILER_FLAGS} ${COMPILER_FLAGS_RELEASE}")
else()
    message(FATAL_ERROR "Unhandled CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
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

option(CPPCHECK "Analyze source using the Cppcheck static code analyzer" ON)
if (CPPCHECK)
    find_program(CMAKE_CXX_CPPCHECK cppcheck)
    if (CMAKE_CXX_CPPCHECK)
        list(
            APPEND CMAKE_CXX_CPPCHECK
            "--std=c++${CMAKE_CXX_STANDARD}"
            "--enable=warning"
            "--inconclusive"
            "--force"
            "--inline-suppr"
            "-Dnodiscard=nodiscard"
            "-DLIKELY=[[likely]]"
            "-DUNLIKELY=[[unlikely]]"
            "-ULRU_CACHE_PRINT_HASH_MAP_IMPLEMENTATION"
            "-DVALUE_TO_STRING"
            "-DVALUE"
            "--error-exitcode=1"
        )
    else()
        message(WARNING "Cppcheck not found. Skipping cppcheck code analyzer")
    endif()
endif()

file(GLOB TESTS ${PROJECT_SOURCE_DIR}/tests/*.cpp)
add_executable(cpp-lru-cache-test ${TESTS})

target_compile_options(cpp-lru-cache-test PRIVATE "SHELL:${COMPILER_FLAGS}")

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

set(CMAKE_CXX_CPPCHECK "")

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    find_program(VALGRIND_EXECUTABLE valgrind)
    if (VALGRIND_EXECUTABLE)
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
        if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
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
endif()
