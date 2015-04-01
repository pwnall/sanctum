#!/bin/sh
#
# Sets up the build system dependencies.

set -o errexit  # Stop the script on the first error.
set -o nounset  # Catch un-initialized variables.

# Check out all the dependencies.
git submodule update --init

# Build and symlink Ninja.
cd deps/ninja
./configure.py --bootstrap
cd ../..
rm -f build/ninja
ln -s "$PWD/deps/ninja/ninja" build/ninja

# LLVM.
mkdir -p build/llvm-build
LLVM_PREFIX="$PWD/build"
cd build/llvm-build
../../deps/llvm/configure --prefix="$LLVM_PREFIX" \
    --enable-cxx11 \
    --enable-optimized --enable-debug-runtime --enable-debug-symbols \
    --with-clang-srcdir=../../deps/clang
make -j4
cd ../..
rm -f build/llvm
ln -s "$PWD/build/llvm-build/Release+Debug+Asserts/" build/llvm
