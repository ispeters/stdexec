#!/usr/bin/env bash

set -uo pipefail

CXX="${CXX:-which clang++}"

echo "Testing with $("$CXX" --version)"

module_src="${1:-foo.cppm}"
test_src="${TEST_SRC:-./main.cpp}"
candidate_pcm="./foo.pcm"

COMMON_FLAGS=(
  -std=c++26
  -Wno-unused-command-line-argument
)

rm -f "$candidate_pcm"

# Step 1: precompile the candidate module against the fixed std.pcm.
precompile_out=$("$CXX" "${COMMON_FLAGS[@]}" \
  --precompile "$module_src" -o "$candidate_pcm" 2>&1)
precompile_status=$?

if [[ $precompile_status -ne 0 ]]; then
  # Module itself doesn't even build -- report distinctly so the
  # interestingness test can tell this apart from "builds fine, no bug".
  echo "PRECOMPILE_FAILED"
  echo "$precompile_out"
  exit 2
fi

# Step 2: compile the fixed, minimized test file against this candidate.
"$CXX" "${COMMON_FLAGS[@]}" \
  -fmodule-file=foo="$candidate_pcm" \
  -fsyntax-only "$test_src" 2>&1
exit $?
