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
# GUARDS ARE COMPILER-ENFORCED, NOT TEXTUAL. Earlier revisions tried to
# reject degenerate reductions by grepping the candidate module's source.
# That approach failed repeatedly for compounding reasons (matching the
# wrong struct by substring, targeting code that wasn't even in the
# reduction target, etc.) and has been abandoned in favour of static_asserts
# in the FIXED test file, which cvise cannot edit. If cvise damages the
# machinery the bug depends on, the assert fails, becomes the first
# `: error:` in the output, and the line-anchored signature check below
# rejects the candidate automatically.
#
# THE REPRO, as of this revision: NOT stdexec's real __tuple or __let/connect
# machinery -- those were eliminated by a step-by-step substitution ladder
# (see git history around "ladder step" commits). What remains:
#   - my_tuple<...>: a bare class template, one static member, nothing else.
#     Confirmed to reproduce the bug exactly as well as __tuple<> does.
#   - stdexec's REAL __variant.hpp (via __let.hpp -- kept unreduced because
#     __variant.hpp relies on meta/concepts machinery it doesn't include
#     itself, and hand-picking a minimal include set risks unrelated errors;
#     let cvise find the minimal subset instead).
#   - A module-side seed: __no_init construction, __emplace_from, __visit
#     over __variant<my_tuple<>> -- confirmed necessary. A bare
#     __variant<my_tuple<>> with no __visit does NOT reproduce it; nor does
#     seeding a DIFFERENT specialization than the importer uses.
#   - The importer performs the exact same sequence, over the exact same
#     specialization, and static_asserts on a member of the visited value.
#
# THE FAILURE: the importer's static_assert fails with the visited value's
# type reported as having no members -- an INCOMPLETENESS diagnosis, not an
# overload-resolution or identity one. same_as-based identity checks between
# the module's and importer's my_tuple<> all pass; the type is found and
# recognised, but its definition is not reachable at the point the module's
# own __visit_alt (instantiated from the importer) needs it.

set -uo pipefail

MODULE_SRC="stdexec.flattened.cppm"
COMPILE="/tmp/reduce-module/compile_module_and_test.sh"

# SIGNATURE is location-free and stays valid wherever the module code lives.
SIGNATURE="cannot be used prior to '::' because it has no members"

# SIGNATURE2 anchors to the FIXED test file's static_assert, never to the
# module -- the module is exactly what's being reduced, so anchoring there
# would break the moment cvise (or a future manual inlining step) shifts it.
SIGNATURE2="/tmp/reduce-module/test_stopped_as_error.cpp:23:23: error: type"

# SIGNATURE3 pins that the incomplete type is specifically my_tuple<> --
# semantic rather than positional, via Clang's own "(aka ...)" desugaring
# text, which is stable across reformatting. Rejects any route to a
# superficially similar diagnostic that isn't actually about this type.
SIGNATURE3="'std::remove_reference_t<decltype(__t)>' (aka 'stdexec::my_tuple<>')"

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

# Every signature must appear in that block. DEBUG_SIGS=1 to see which one
# rejected a candidate.
i=0
for sig in "$SIGNATURE" "$SIGNATURE2" "$SIGNATURE3"; do
  i=$((i + 1))
  if ! printf '%s' "$first_error_block" | grep -qF "$sig"; then
    [[ -n "${DEBUG_SIGS:-}" ]] && echo "Signature $i didn't match" >&2
    exit 1
  fi
done

[[ -n "${DEBUG_SIGS:-}" ]] && echo "All signatures matched" >&2
exit 0
