#!/usr/bin/env bash
#
# compile_module_and_test.sh -- given a candidate (possibly cvise-reduced)
# flattened module interface file, precompile it fresh, then compile the
# FIXED, already-minimized test_stopped_as_error.cpp against that candidate
# module. Unlike compile_one.sh, nothing here is cached across iterations --
# we genuinely need to know whether THIS candidate module still exhibits the
# bug when the (fixed) test file imports it.
#
# Usage: ./compile_module_and_test.sh <candidate-module-file>
#
# Expects, in the same directory:
#   - the candidate module file (arg $1, e.g. stdexec.flattened.cppm)
#   - test_stopped_as_error.cpp  (the already-minimized file from the first
#     cvise pass -- kept fixed, never touched by this reduction run)

set -uo pipefail

STDEXEC_ROOT="${STDEXEC_ROOT:-/Users/ianpetersen/git/stdexec}"
STD_PCM="${STD_PCM:-/Users/ianpetersen/git/stdexec/build/CMakeFiles/__cmake_cxx23.dir/std.pcm}"
CXX="${CXX:-/nix/store/hgs9vcsaa3dw51gn09kh5jf647ar3ybh-clang-wrapper-22.1.5/bin/clang++}"

module_src="$1"
test_src="${TEST_SRC:-/tmp/reduce-module/test_stopped_as_error.cpp}"
candidate_pcm="./stdexec_candidate.pcm"

COMMON_FLAGS=(
  -std=gnu++23
  -DSTDEXEC_BUILD_MODULES=1
  -DSTDEXEC_ENABLE_LIBDISPATCH
  -DSTDEXEC_NAMESPACE=std::execution
  -arch arm64
  -isysroot /nix/store/byqf5h6mkq7fd8f1z7dvas3rv3z4v7l9-apple-sdk-14.4/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk
  -isystem /Users/ianpetersen/git/stdexec/include/stdexec/__detail
  -isystem /Users/ianpetersen/git/stdexec/include/stdexec
  -isystem /Users/ianpetersen/git/stdexec/include/exec
  -isystem /Users/ianpetersen/git/stdexec/include
  -mmacosx-version-min=14.0
  -Wno-c2y-extensions
  -Wno-gnu-line-marker
  -ferror-limit=0
  -fmacro-backtrace-limit=0
  -ftemplate-backtrace-limit=0
)

rm -f "$candidate_pcm"

# Step 1: precompile the candidate module against the fixed std.pcm.
precompile_out=$("$CXX" "${COMMON_FLAGS[@]}" \
  -fmodule-file=std="$STD_PCM" \
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
  -fmodule-file=std="$STD_PCM" \
  -fmodule-file=foo="$candidate_pcm" \
  -fsyntax-only -c "$test_src" 2>&1
exit $?
