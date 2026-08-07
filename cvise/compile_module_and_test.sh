#!/usr/bin/env bash

set -uo pipefail

CXX="${CXX:-clang++}"

echo "Testing with $("$CXX" --version)"

FLAGS=(-std=c++26 -Wno-unused-command-line-argument)

rm -f ./foo.pcm

# Step 1: precompile the module interface.
"$CXX" "${FLAGS[@]}" --precompile foo.cppm -o ./foo.pcm || exit $?

# Step 2: compile the importer against it.  One diagnostic is expected, on the
# last line of main.cpp; every other line in main() should compile clean.
echo
echo "=== modules build (one error expected) ==="
"$CXX" "${FLAGS[@]}" -fmodule-file=foo=./foo.pcm -fsyntax-only main.cpp
modules_status=$?

# Step 3: the same code with no module syntax at all.
echo
echo "=== non-modular control (no diagnostics expected) ==="
"$CXX" "${FLAGS[@]}" -fsyntax-only no_modules.cpp
control_status=$?

echo
echo "modules build exited $modules_status; non-modular control exited $control_status"
