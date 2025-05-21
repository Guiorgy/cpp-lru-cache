#!/bin/sh

cd "$(dirname -- "$0")"

BUILD_TYPES='Debug Release'
HASH_MAP_IMPLEMENTATIONS='STL STL_PMR ABSEIL TESSIL ANKERL ANKERL_SEG'

build_and_run_tests() {
  local build_type="$1"
  local hm_impl="$2"

  local build_dir="build/$build_type"
  mkdir -p "$build_dir"
  cd "$build_dir"

  echo "Configuring the configuration: $build_type $hm_impl"
  cmake_output=$(cmake -DCMAKE_BUILD_TYPE=$build_type -DHASH_MAP_IMPLEMENTATION=$hm_impl ../.. 2>&1)
  if [ $? -ne 0 ]; then
    echo "$cmake_output"
    echo "CMake configuration failed for the configuration: $build_type $hm_impl"
    exit 1
  fi

  echo "Compiling the configuration: $build_type $hm_impl"
  make_output=$(make -j$(nproc) all 2>&1)
  if [ $? -ne 0 ]; then
    echo "$cmake_output"
    echo "$make_output"
    echo "Compilation failed for the configuration: $build_type $hm_impl"
    exit 1
  fi

  echo "Running tests for the configuration: $build_type $hm_impl"
  test_output=$(make test 2>&1)
  if [ $? -ne 0 ]; then
    echo "$cmake_output"
    echo "$make_output"
    echo "$test_output"
    echo "Tests failed for the configuration: $build_type $hm_impl"
    exit 1
  fi

  echo "Running sanitizers for the configuration: $build_type $hm_impl"
  sanitize_output=$(make sanitize 2>&1)
  if [ $? -ne 0 ]; then
    echo "$cmake_output"
    echo "$make_output"
    echo "$test_output"
    echo "$sanitize_output"
    echo "Sanitizer tests failed for the configuration: $build_type $hm_impl"
    exit 1
  fi

  cd ../..

  echo '------------------------------------------------'
}

for build_type in $BUILD_TYPES; do
  for hm_impl in $HASH_MAP_IMPLEMENTATIONS; do
    build_and_run_tests "$build_type" "$hm_impl"
  done
done

echo "All configuration tests passed"
