#!/bin/sh

cd "$(dirname -- "$0")"

if command -v -- unbuffer > /dev/null 2>&1; then
  UNBUFFER='unbuffer'
else
  echo 'Command "unbuffer" not found. Output may not be colored' >&2

  if command -v -- apt > /dev/null 2>&1; then
    echo 'Try running "sudo apt install -y expect"' >&2
  fi

  UNBUFFER=''
fi

BUILD_TYPES='Debug Release'
HASH_MAP_IMPLEMENTATIONS='STD ABSEIL TESSIL_SPARSE TESSIL_ROBIN TESSIL_HOP ANKERL ANKERL_SEG'

if [ $# -ne 0 ] && [ "$1" = 'dev' ]; then
  DEV=1
else
  DEV=0
fi

build_and_run_tests() {
  local build_type="$1"
  local hm_impl="$2"

  local build_dir="build/$build_type"
  mkdir -p "$build_dir"
  cd "$build_dir"

  local cmake_args="-DCMAKE_BUILD_TYPE=$build_type -DCPPCHECK=ON -DHASH_MAP_IMPLEMENTATION=$hm_impl"
  if [ $DEV -eq 1 ]; then
    cmake_args="$cmake_args -Wdev -Werror=dev -Wdeprecated -Werror=deprecated --warn-uninitialized --loglevel=VERBOSE"
  fi

  echo "Configuring the configuration: $build_type $hm_impl"
  cmake_output=$($UNBUFFER cmake $cmake_args ../.. 2>&1)
  if [ $? -ne 0 ]; then
    echo "$cmake_output"
    echo "CMake configuration failed for the configuration: $build_type $hm_impl"
    exit 1
  fi

  echo "Compiling the configuration: $build_type $hm_impl"
  make_output=$($UNBUFFER make -j$(nproc) all 2>&1)
  if [ $? -ne 0 ]; then
    echo "$cmake_output"
    echo "$make_output"
    echo "Compilation failed for the configuration: $build_type $hm_impl"
    exit 1
  fi

  echo "Running tests for the configuration: $build_type $hm_impl"
  test_output=$($UNBUFFER make test 2>&1)
  if [ $? -ne 0 ]; then
    echo "$cmake_output"
    echo "$make_output"
    echo "$test_output"
    echo "Tests failed for the configuration: $build_type $hm_impl"
    exit 1
  fi

  echo "Running sanitizers for the configuration: $build_type $hm_impl"
  sanitize_output=$($UNBUFFER make sanitize 2>&1)
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

echo 'All configuration tests passed'
