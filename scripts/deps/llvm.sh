#!/bin/sh
#
# Builds and symlinks LLVM.

set -o errexit  # Stop the script on the first error.
set -o nounset  # Catch un-initialized variables.

mkdir -p build/llvm-build
cd build/llvm-build

# NOTE: CMake's ninja generator barfs if it can't find ninja on the path.
PATH="$PATH:$PWD/../" ../cmake -G "Ninja" \
    -DCMAKE_BUILD_TYPE:STRING=RelWithDebInfo \
    -DLLVM_ENABLE_RTTI:BOOL=ON \
    -DLLVM_INCLUDE_EXAMPLES:BOOL=OFF \
    -DLLVM_INCLUDE_TESTS:BOOL=OFF \
    -DLLVM_EXTERNAL_CLANG_SOURCE_DIR:PATH="$PWD/../../deps/clang" \
    ../../deps/llvm
../ninja

cd ../..
rm -f build/llvm
ln -s "$PWD/build/llvm-build/Release+Debug+Asserts/" build/llvm
