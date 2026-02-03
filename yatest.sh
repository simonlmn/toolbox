#!/bin/bash
# Build and run tests for a library.
# 
# Copy this file into the root of your library and run it to build and execute tests.
#

set -e

LIB_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

export ARDUINO_DIRECTORIES_DATA="$LIB_DIR/build/.arduino"
export ARDUINO_DIRECTORIES_DOWNLOADS="$LIB_DIR/build/.arduino/staging"
export ARDUINO_DIRECTORIES_USER="$LIB_DIR/build/.arduino"

if [ -z "$YATEST_DIR" -o ! -d "$YATEST_DIR" ]; then
    echo "YATEST_DIR is not set or does not point to a directory. Installing yatest library..."
    arduino-cli lib update-index
    arduino-cli lib install yatest

    YATEST_DIR="$(cd "$ARDUINO_DIRECTORIES_USER/libraries/yatest" && pwd)"
    if [ ! -f "$YATEST_DIR/src/build-and-run.sh" ]; then
        echo "Error: yatest build script not found at $YATEST_DIR"
        exit 1
    fi
fi

# Once yatest is installed, we can use the build-and-run.sh script from there to take over the rest.
"$YATEST_DIR/src/build-and-run.sh" "$LIB_DIR"
