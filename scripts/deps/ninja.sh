#!/bin/sh
#
# Builds and symlinks Ninja.

set -o errexit  # Stop the script on the first error.
set -o nounset  # Catch un-initialized variables.

# NOTE: Ninja really wants to be built inside its source tree.
cd deps/ninja

./configure.py --bootstrap
rm -f null.o  # NOTE: This gets created for some weird reason.

cd ../..
rm -f build/ninja
ln -s "$PWD/deps/ninja/ninja" build/ninja
