#!/bin/sh

PROJECT_PATH="$(dirname -- "$0")"

if command -v -- unbuffer > /dev/null 2>&1; then
  UNBUFFER='unbuffer'
else
  echo 'Command "unbuffer" not found. Output may not be colored' >&2

  if command -v -- apt > /dev/null 2>&1; then
    echo 'Try running "sudo apt install -y expect"' >&2
  fi

  UNBUFFER=''
fi

CXX_COMPILERS='g++ clang'
BUILD_TYPES='Debug Release'
HASH_MAP_IMPLEMENTATIONS='STD ABSEIL TESSIL_SPARSE TESSIL_ROBIN TESSIL_HOP ANKERL ANKERL_SEG'

if [ $# -ne 0 ] && [ "$1" = 'dev' ]; then
  DEV=1
else
  DEV=0
fi

build_and_run_tests() {
  local cxx_compiler="$1"
  local build_type="$2"
  local hash_map_impl="$3"
  local config="$cxx_compiler $build_type $hash_map_impl"

  local build_dir="$PROJECT_PATH/build/$cxx_compiler/$build_type"
  mkdir -p "$build_dir"
  cd "$build_dir"

  local cmake_args="-DCMAKE_CXX_COMPILER=$cxx_compiler -DCMAKE_BUILD_TYPE=$build_type -DCPPCHECK=ON -DHASH_MAP_IMPLEMENTATION=$hash_map_impl"
  if [ $DEV -eq 1 ]; then
    cmake_args="$cmake_args -Wdev -Werror=dev -Wdeprecated -Werror=deprecated --warn-uninitialized --loglevel=VERBOSE"
  fi

  echo "Configuring the configuration: $config"
  cmake_output=$($UNBUFFER cmake $cmake_args ../../.. 2>&1)
  if [ $? -ne 0 ]; then
    echo "$cmake_output"
    echo "CMake configuration failed for the configuration: $config"
    exit 1
  fi

  echo "Compiling the configuration: $config"
  make_output=$($UNBUFFER make -j$(nproc) all 2>&1)
  if [ $? -ne 0 ]; then
    echo "$cmake_output"
    echo "$make_output"
    echo "Compilation failed for the configuration: $config"
    exit 1
  fi

  echo "Running tests for the configuration: $config"
  test_output=$($UNBUFFER make test 2>&1)
  if [ $? -ne 0 ]; then
    echo "$cmake_output"
    echo "$make_output"
    echo "$test_output"
    echo "Tests failed for the configuration: $config"
    exit 1
  fi

  echo "Running sanitizers for the configuration: $config"
  sanitize_output=$($UNBUFFER make sanitize 2>&1)
  if [ $? -ne 0 ]; then
    echo "$cmake_output"
    echo "$make_output"
    echo "$test_output"
    echo "$sanitize_output"
    echo "Sanitizer tests failed for the configuration: $config"
    exit 1
  fi

  echo '------------------------------------------------'
}

for cxx_compiler in $CXX_COMPILERS; do
  if !command -v -- "$cxx_compiler" > /dev/null 2>&1; then
    echo "Skipping the missing compiler: $cxx_compiler" >&2
    continue
  fi

  for build_type in $BUILD_TYPES; do
    for hash_map_impl in $HASH_MAP_IMPLEMENTATIONS; do
      build_and_run_tests "$cxx_compiler" "$build_type" "$hash_map_impl"
    done
  done
done

echo 'All configuration tests passed'
