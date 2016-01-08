#!/bin/sh
#
# Sets up the build system dependencies.

set -o errexit  # Stop the script on the first error.
set -o nounset  # Catch un-initialized variables.

# NOTE: All the scripts below also need to be listed in .travis.yml
#       We cannot call all.sh directly in .travis.yml, because that would
#       exceed the 50-minute time limit for a single build step.

./scripts/deps/setup.sh
./scripts/deps/ninja.sh
./scripts/deps/cmake.sh
./scripts/deps/llvm.sh
