#!/usr/bin/env bash
#
# interestingness_test_module.sh -- for use with cvise reducing
# stdexec.flattened.cppm, with test_stopped_as_error.cpp held fixed.
#
# Usage (one-time setup):
#   ./flatten_module.sh > stdexec.flattened.cppm
#   mkdir /tmp/reduce-module && cd /tmp/reduce-module
#   cp /path/to/stdexec.flattened.cppm .
#   cp /path/to/compile_module_and_test.sh .
#   cp /path/to/interestingness_test_module.sh .
#   cp /path/to/test_stopped_as_error.cpp .     # the already-minimized one
#   chmod +x compile_module_and_test.sh interestingness_test_module.sh
#   ./interestingness_test_module.sh            # sanity check -- must exit 0
#   cvise interestingness_test_module.sh stdexec.flattened.cppm
#
# IMPORTANT: cvise copies ONLY the reduction target (stdexec.flattened.cppm)
# into a fresh, isolated directory each round -- so MODULE_SRC below MUST
# stay a relative path (it genuinely lives in cvise's current directory).
# Everything else (this script's helper, the fixed test file) is NOT
# copied into that isolated directory, so those must be absolute paths --
# see TEST_SRC's default in compile_module_and_test.sh.
#
# SOURCE-LEVEL GUARDS: the reduction has now converged on the same
# degenerate false positive TWICE -- shrinking __apply_t::__impl's
# operator() down to a zero-parameter, unconditionally-uncallable overload
# (`template <__callable> void operator()();`), which reprints matching
# diagnostic text without exercising the real cvref/tuple-identity
# mechanism at all. A previous line-window-based grep for this missed it
# once already because it's sensitive to exactly how many lines of
# whitespace/formatting sit between `struct __impl` and the declaration --
# so this version extracts the __impl struct's FULL body via brace-depth
# tracking (immune to reformatting) and checks it directly:
#   - BLACKLIST: reject if that body contains `operator()()` (whitespace
#     stripped first, so no formatting trick can hide it)
#   - WHITELIST: require that body still mentions `_Tuple` at all -- the
#     real overload cannot exist without deducing something tuple-shaped,
#     so its total absence is itself suspicious regardless of the exact
#     shape cvise arrived at.

set -uo pipefail

MODULE_SRC="stdexec.flattened.cppm"
COMPILE="/tmp/reduce-module/compile_module_and_test.sh"

SIGNATURE="no matching function for call to object of type 'stdexec::__tup::__apply_t::__impl<stdexec::__cplr>'"
SIGNATURE2="/tmp/reduce-module/test_stopped_as_error.cpp:100:31: note: while substituting deduced template arguments"
SIGNATURE3="100 |     auto op = stdexec::connect(snd, rcvr{});"

# --- Extract the __impl struct's full body via brace-depth tracking.
impl_body=$(awk '
  /struct __impl/ { capturing=1 }
  capturing {
    print
    opens  = gsub(/{/, "{")
    closes = gsub(/}/, "}")
    depth += opens - closes
    if (depth <= 0 && (opens + closes) > 0) { exit }
  }
' "$MODULE_SRC" 2>/dev/null)

# --- Guard A (blacklist): reject the known degenerate zero-parameter shape,
# checked with all whitespace stripped so no reformatting can dodge it.
impl_body_nowhitespace=$(printf '%s' "$impl_body" | tr -d '[:space:]')
if printf '%s' "$impl_body_nowhitespace" | grep -qF 'operator()()'; then
  #echo "Guard A failed"
  true #exit 1
fi

# --- Guard B (whitelist): require the block still mentions __tuple_t --
# the alias defined *inside* __impl itself (`template <class... _Ts> using
# __tuple_t = __mcall1<_CvRef, __tuple<_Ts...>>;`) that the real deduction
# mechanism depends on. NOTE: an earlier version of this guard checked for
# "_Tuple" instead, but that identifier only appears OUTSIDE __impl (in the
# outer __apply_t::operator() and the __impl_t alias) -- it was never
# present inside __impl's own body even in genuine, unreduced source, so
# that version of the guard rejected everything unconditionally.
if ! printf '%s' "$impl_body" | grep -qF '__tuple_t'; then
  #echo "Guard B failed"
  true #exit 1
fi

out=$("$COMPILE" "$MODULE_SRC" 2>&1)
status=$?

if [[ $status -eq 0 ]]; then
  # Test file compiled clean against this candidate module -- the bug
  # got reduced away.
  #echo "Clean compile"
  exit 1
fi

if [[ $status -eq 2 ]]; then
  # Candidate module doesn't even precompile -- also not interesting.
  #echo "Module doesn't compile"
  exit 1
fi

# --- Guard C: reject the "body deleted, call site survived" failure mode.
if printf '%s' "$out" | grep -q "cannot be used before it is defined"; then
  #echo "Guard C failed"
  exit 1
fi

# Isolate the first error's diagnostic block (error line + its notes),
# same reasoning as the first reduction pass: the real failure legitimately
# recurs, and error-recovery on a broken candidate module could coincidentally
# reach our signature text later on without it being the FIRST error.
first_error_block=$(printf '%s\n' "$out" | awk '
  /: error:/ { n++ }
  n==1       { print }
  n==2       { exit }
')

if printf '%s' "$first_error_block" | grep -qF "$SIGNATURE"; then
  if printf '%s' "$first_error_block" | grep -qF "$SIGNATURE2"; then
    if printf '%s' "$first_error_block" | grep -qF "$SIGNATURE3"; then
      #echo "All three signatures matched"
      exit 0
    else
      #echo "Signature 3 didn't match"
      exit 1
    fi
  else
    #echo "Signature 2 didn't match"
    exit 1
  fi
else
  #echo "Signature 1 didn't match"
  exit 1
fi

exit 1
