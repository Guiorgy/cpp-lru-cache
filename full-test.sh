#!/bin/sh

cd "$(dirname -- "$0")"

BUILD_TYPES='Debug Release'
OPTIONS='PMR ABSEIL TESSIL ANKERL ANKERL_SEG'

build_and_run_tests() {
  local build_type="$1"
  local enabled_option="$2"

  local build_dir="build/$build_type"
  mkdir -p "$build_dir"
  cd "$build_dir"

  local cmake_args=''
  for option in $OPTIONS; do
    if [ "$option" != "$enabled_option" ]; then
      cmake_args="$cmake_args -D${option}=OFF"
    fi
  done
  if [ -n "$enabled_option" ]; then
    cmake_args="$cmake_args -D$enabled_option=ON"
  fi

  echo "Configuring the configuration: $build_type $enabled_option"
  cmake_output=$(cmake $cmake_args ../.. 2>&1)
  if [ $? -ne 0 ]; then
    echo "$cmake_output"
    echo "CMake configuration failed for the configuration: $build_type $enabled_option"
    exit 1
  fi

  echo "Compiling the configuration: $build_type $enabled_option"
  make_output=$(make -j$(nproc) all 2>&1)
  if [ $? -ne 0 ]; then
    echo "$cmake_output"
    echo "$make_output"
    echo "Compilation failed for the configuration: $build_type $enabled_option"
    exit 1
  fi

  echo "Running tests for the configuration: $build_type $enabled_option"
  test_output=$(make test 2>&1)
  if [ $? -ne 0 ]; then
    echo "$cmake_output"
    echo "$make_output"
    echo "$test_output"
    echo "Tests failed for the configuration: $build_type $enabled_option"
    exit 1
  fi

  echo "Running sanitizers for the configuration: $build_type $enabled_option"
  sanitize_output=$(make sanitize 2>&1)
  if [ $? -ne 0 ]; then
    echo "$cmake_output"
    echo "$make_output"
    echo "$test_output"
    echo "$sanitize_output"
    echo "Sanitizer tests failed for the configuration: $build_type $enabled_option"
    exit 1
  fi

  cd ../..

  echo '------------------------------------------------'
}

for build_type in $BUILD_TYPES; do
  build_and_run_tests "$build_type" ""

  for option in $OPTIONS; do
    build_and_run_tests "$build_type" "$option"
  done
done

echo "All configuration tests passed"
