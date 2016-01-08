#!/bin/sh
#
# Builds and symlinks CMake.

set -o errexit  # Stop the script on the first error.
set -o nounset  # Catch un-initialized variables.

mkdir -p build/cmake-build
cd build/cmake-build
../../deps/cmake/bootstrap
make

cd ../..
rm -f build/cmake
ln -s "$PWD/build/cmake-build/bin/cmake" build/
