#!/bin/sh
#
# Project-specific setup.

set -o errexit  # Stop the script on the first error.
set -o nounset  # Catch un-initialized variables.

# Check out all the build dependencies stored in Git submodules.
git submodule update --init

# Symlink build scripts in the build directory.
mkdir -p build/
ln -s "$PWD/scripts/run_gyp.sh" build/run_gyp.sh
