#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"

cmake -S "$SCRIPT_DIR" -B "$BUILD_DIR"
cmake --build "$BUILD_DIR" --target prosched_tests

set +e
"$BUILD_DIR/prosched/prosched_tests" --gtest_color=yes
EXIT_CODE=$?

exit $EXIT_CODE
