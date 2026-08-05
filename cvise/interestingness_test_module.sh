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
# GUARDS ARE NOW COMPILER-ENFORCED, NOT TEXTUAL. Earlier revisions tried to
# reject degenerate reductions by grepping the candidate module's source for
# the shape of __apply_t::__impl. That approach failed twice and for two
# compounding reasons: (1) `/struct __impl/` matches `struct __impls` in
# namespace __let, which appears earlier in the file, so the awk extracted the
# wrong struct entirely; and (2) __apply_t lives in __tuple.hpp, which is not
# in the reduction target at all, so there was never anything to find.
#
# All of that is now handled by static_asserts in the FIXED test file, which
# cvise cannot edit. If cvise damages the machinery the bug depends on, one of
# those asserts fails, becomes the first `: error:` in the output, and the
# line-anchored signature check below rejects the candidate automatically --
# no extra logic needed here. The compiler decides what "still meaningful"
# means, rather than an awk script guessing at source shape.
#
# TRIGGER: the failure is now provoked by `static_assert(sizeof(opstate2) > 0)`
# rather than by a call to connect(). Completing that class instantiates the
# virtual override __start_next (Clang instantiates virtual bodies at
# class-completion time even under -fsyntax-only), which is a far shorter
# instantiation chain -- the whole connect/__sexpr/transform_sender_t subtree
# no longer has to survive reduction.

set -uo pipefail

MODULE_SRC="stdexec.flattened.cppm"
COMPILE="/tmp/reduce-module/compile_module_and_test.sh"

# SIGNATURE is location-free and stays valid wherever the module code lives.
SIGNATURE="no matching function for call to object of type 'stdexec::__tup::__apply_t::__impl<stdexec::__cplr>'"

# SIGNATURE2/3 anchor to the FIXED test file, never to the module. The primary
# error's own file/line migrates as headers get inlined into the flattened
# module between rounds (it was __let.hpp:292 at last check, and moves into
# stdexec.flattened.cppm once __let.hpp is inlined) -- anchoring there would
# silently stop matching at exactly that transition, which looks like cvise
# mysteriously stalling rather than like a broken test.
SIGNATURE2="/tmp/reduce-module/test_stopped_as_error.cpp:117:17: note: in instantiation of member function"
SIGNATURE3="117 |   static_assert(sizeof(opstate2) > 0);"

# SIGNATURE4 is semantic rather than positional: it pins that the class being
# completed is the EMPTY-tuple opstate and that __start_next is what is being
# instantiated. Immune to file moves and to cvise reformatting, and it rejects
# any route to the diagnostic that doesn't go through the empty-tuple opstate.
SIGNATURE4="stdexec::__tup::__tuple<>>::__start_next"

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

# Every signature must appear in that block. Flattened to a loop now that
# there are four of them; set DEBUG_SIGS=1 to see which one rejected.
i=0
for sig in "$SIGNATURE" "$SIGNATURE2" "$SIGNATURE3" "$SIGNATURE4"; do
  i=$((i + 1))
  if ! printf '%s' "$first_error_block" | grep -qF "$sig"; then
    [[ -n "${DEBUG_SIGS:-}" ]] && echo "Signature $i didn't match" >&2
    exit 1
  fi
done

[[ -n "${DEBUG_SIGS:-}" ]] && echo "All signatures matched" >&2
exit 0
