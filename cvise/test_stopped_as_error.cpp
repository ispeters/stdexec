import stdexec;
import std;

// Canary: the empty-tuple fast-path overloads of __apply_t must still be
// selectable by overload resolution. If cvise collapses them into the
// degenerate zero-parameter shape we've seen twice, this becomes ill-formed
// -- a hard, precisely-located compile error, not text cvise can dodge by
// reprinting similar-looking diagnostic text from an unrelated path.
// NOTE: near-inert while __tuple.hpp is not yet inlined into the reduction
// target -- these currently test the real, unreduced __apply_t. They go live
// the moment __tuple.hpp gets flattened in.
static_assert(
    stdexec::__applicable<decltype([](auto &&...) noexcept { return 0; }),
                          stdexec::__tuple<>>);

// Canary: the generic, constrained path (through __apply_t::__impl<_CvRef>
// and its __tuple_t alias) must still work for a *non-empty* tuple.
// __tuple<int> is a distinct partial specialization from __tuple<>, so this
// exercises the general mechanism independent of the specific bug being
// chased, and would catch cvise damaging __impl's constraint/deduction
// machinery generally.
static_assert(stdexec::__applicable<decltype([](int) { return 0; }),
                                    stdexec::__tuple<int>>);

struct rcvr {
  using receiver_concept = stdexec::receiver_tag;

  constexpr void set_value(auto &&...) && noexcept {}
};

int main() {
  auto lambda = [](auto &...) noexcept { return stdexec::just(2); };

  // Pin both child senders' descriptor shape. The suspected identity split
  // puts __tuple<> in two distinct positions: as the _Tuples... element of
  // __opstate, and inside the child's __desc<just_t, __tuple<>>{} class-type
  // NTTP. With no connect call left to keep __sexpr honest, these are what
  // stop cvise hollowing out the descriptor and quietly deleting the second
  // occurrence. They replace the old structured-binding asserts and need no
  // sender object to exist.
  static_assert(std::same_as<stdexec::tag_of_t<decltype(stdexec::just(1))>,
                             stdexec::just_t>);
  static_assert(std::same_as<stdexec::__data_of<decltype(stdexec::just(1))>,
                             stdexec::__tuple<int>>);
  static_assert(std::same_as<stdexec::__data_of<decltype(stdexec::just())>,
                             stdexec::__tuple<>>);

  using opstate1 =
      stdexec::__let::__opstate<stdexec::set_value_t,
                                decltype(stdexec::just(1)) &, decltype(lambda),
                                rcvr, stdexec::__tuple<int>>;

  // Mirror control: the NON-empty path must complete cleanly. This is the
  // other half of the empty-vs-non-empty discriminator -- without it, a
  // candidate module in which *everything* fails to instantiate would still
  // look interesting. (Strictly redundant with the member-typedef aliases
  // below, which already force completion, but stated explicitly so it
  // survives if those aliases are ever dropped.)
  static_assert(sizeof(opstate1) > 0);

  using storage1_t = opstate1::__op_state_variant_t;
  using env21_t = opstate1::__env2_t;

  static_assert(stdexec::__callable<decltype(stdexec::__let::__start_next_fn),
                                    decltype(lambda) &, rcvr &, env21_t &,
                                    storage1_t &, stdexec::__tuple<int> &>);
  static_assert(stdexec::__callable<decltype(stdexec::__let::__mk_result_sndr),
                                    decltype(lambda) &>);

  // Pin __mk_result_sndr's parameter shape. The real lambda is
  // []<class _Fun, class... _Args>(_Fun& __fn, _Args&... __args) -- every
  // parameter an lvalue reference -- so an rvalue _Fun must NOT bind. The
  // by-value C-varargs form cvise collapsed to last run
  // (`[]<class _Fun>(_Fun __fn...)`) accepts an rvalue here AND makes
  // __callable trivially true for any trailing arguments, defusing the very
  // constraint chain the bug lives in.
  static_assert(!stdexec::__callable<decltype(stdexec::__let::__mk_result_sndr),
                                     decltype(lambda)>);

  static_assert(
      std::same_as<
          decltype(stdexec::just(2)),
          stdexec::__invoke_result_t<decltype(stdexec::__let::__mk_result_sndr),
                                     decltype(lambda) &>>);
  static_assert(stdexec::__callable<
                stdexec::__tup::__apply_t::__impl_t<stdexec::__tuple<int> &>,
                decltype(stdexec::__let::__mk_result_sndr) &,
                stdexec::__tuple<int> &, decltype(lambda) &>);
  static_assert(std::same_as<stdexec::__variant<stdexec::__tuple<int>>,
                             decltype(opstate1::__args_)>);

  // Direct probe: the same constraint the compiler reports as "evaluated to
  // false" when it is reached through __opstate::__start_next. The `const`
  // that used to be here is deliberately gone -- the in-context instantiation
  // deduces a non-const _Fn, so without it this is the *same* specialization
  // rather than a near neighbour, and the direct-vs-contextual contrast is
  // airtight: identical constraint, satisfied here, unsatisfied there.
  static_assert(stdexec::__callable<
                stdexec::__tup::__apply_t::__impl_t<stdexec::__tuple<> &>,
                decltype(stdexec::__let::__mk_result_sndr) &,
                stdexec::__tuple<> &, decltype(lambda) &>);
  stdexec::__tuple<> empty;
  using apply_t = decltype(stdexec::__apply(stdexec::__let::__mk_result_sndr,
                                            empty, lambda));

  static_assert(std::same_as<apply_t, decltype(stdexec::just(2))>);

  using opstate2 =
      stdexec::__let::__opstate<stdexec::set_value_t,
                                decltype(stdexec::just()) &, decltype(lambda),
                                rcvr, stdexec::__tuple<>>;

  // THE TRIGGER. Completing this class instantiates the virtual override
  // __start_next -- Clang instantiates virtual bodies at class-completion
  // time even under -fsyntax-only -- which evaluates the __apply constraint
  // asserted above, in a context where it comes out false.
  static_assert(sizeof(opstate2) > 0);
}
