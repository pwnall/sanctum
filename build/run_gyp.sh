#!/bin/sh
#
# Rebuilds ninja files out of gyp files.

set -o errexit  # Stop the script on the first error.
set -o nounset  # Catch un-initialized variables.

deps/gyp/gyp \
    --format=ninja \
    --toplevel-dir=. \
    src/main.gyp
