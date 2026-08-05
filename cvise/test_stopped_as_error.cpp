import stdexec;
import std;

// Canary: the empty-tuple fast-path overloads of __apply_t must still be
// selectable by overload resolution. If cvise collapses them into the
// degenerate zero-parameter shape we've seen twice, this becomes ill-formed
// -- a hard, precisely-located compile error, not text cvise can dodge by
// reprinting similar-looking diagnostic text from an unrelated path.
static_assert(
    stdexec::__applicable<decltype([](auto &&...) noexcept { return 0; }),
                          stdexec::__tuple<>>);

// Canary: the generic, constrained path (through __apply_t::__impl<_CvRef>
// and its __tuple_t alias) must still work for a *non-empty* tuple.
// __tuple<int> is a distinct partial specialization from __tuple<>, so this
// exercises the general mechanism independent of the specific bug you're
// chasing, and would catch cvise damaging __impl's constraint/deduction
// machinery generally.
static_assert(stdexec::__applicable<decltype([](int) { return 0; }),
                                    stdexec::__tuple<int>>);

struct rcvr {
  using receiver_concept = stdexec::receiver_tag;

  constexpr void set_value(auto &&...) && noexcept {}
};

int main() {
  auto lambda = [](auto &...) noexcept { return stdexec::just(2); };

  using opstate1 =
      stdexec::__let::__opstate<stdexec::set_value_t,
                                decltype(stdexec::just(1)) &, decltype(lambda),
                                rcvr, stdexec::__tuple<int>>;
  using storage1_t = opstate1::__op_state_variant_t;
  using env21_t = opstate1::__env2_t;

  static_assert(stdexec::__callable<decltype(stdexec::__let::__start_next_fn),
                                    decltype(lambda) &, rcvr &, env21_t &,
                                    storage1_t &, stdexec::__tuple<int> &>);
  static_assert(stdexec::__callable<decltype(stdexec::__let::__mk_result_sndr),
                                    decltype(lambda) &>);
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

  {
    auto snd = stdexec::let_value(stdexec::just(1), lambda);

    static_assert(sizeof(snd) == 4);

    auto &[tag, data, child] = snd;

    static_assert(std::same_as<decltype(tag), stdexec::let_value_t>);
    static_assert(std::same_as<decltype(data), decltype(lambda)>);

    auto &[tag2, data2] = child;

    static_assert(std::same_as<decltype(tag2), stdexec::just_t>);
    auto &[value] = data2;
    static_assert(std::same_as<decltype(value), int>);

    using fn_t = decltype(stdexec::__let::__mk_result_sndr) &;
    using tuple_t = decltype(value) &;
    using us_t = decltype(lambda) &;

    auto op = stdexec::connect(snd, rcvr{});

    static_assert(std::same_as<decltype(op), opstate1>);
    op.start();
  }

  static_assert(stdexec::__callable<
                stdexec::__tup::__apply_t::__impl_t<stdexec::__tuple<> &>,
                const decltype(stdexec::__let::__mk_result_sndr) &,
                struct stdexec::__tup::__tuple<> &, decltype(lambda) &>);
  stdexec::__tuple<> empty;
  using apply_t = decltype(stdexec::__apply(stdexec::__let::__mk_result_sndr,
                                            empty, lambda));

  static_assert(std::same_as<apply_t, decltype(stdexec::just(2))>);

  using opstate2 =
      stdexec::__let::__opstate<stdexec::set_value_t,
                                decltype(stdexec::just()) &, decltype(lambda),
                                rcvr, stdexec::__tuple<>>;
  // using env22_t = opstate2::__env2_t;

#if 1
  {
    auto snd = stdexec::let_value(stdexec::just(), lambda);
    auto op = stdexec::connect(snd, rcvr{});
    op.start();
  }
#endif
}
