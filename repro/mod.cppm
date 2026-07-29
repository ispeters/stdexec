module;

export module repro;

import std;

namespace ns
{
  //////////////////////////////////////////////////////////////////////////
  // cvref-tag dispatch, mirroring __cp/__cplr/... + __copy_cvref_fn
  export struct cp
  {
    template <class Tp>
    using f = Tp;
  };

  export struct cplr
  {
    template <class Tp>
    using f = Tp&;
  };

  template <class>
  extern cp cpcvr;
  template <class Tp>
  extern cplr cpcvr<Tp&>;

  template <class Tp>
  using copy_cvref_fn = decltype(cpcvr<Tp>);

  template <class Fn, class Arg>
  using mcall1 = Fn::template f<Arg>;

  //////////////////////////////////////////////////////////////////////////
  // tuple: primary template built on a base, PLUS an explicit full
  // specialization for the empty pack (mirrors stdexec's __tupl_base /
  // __tuple<> split)
  template <class... Ts>
  struct tupl_base
  {
    static constexpr std::size_t size = sizeof...(Ts);
  };

  export template <class... Ts>
  struct tuple : tupl_base<Ts...>
  {};

  export template <>
  struct tuple<>
  {
    static constexpr std::size_t size = 0;
  };

  //////////////////////////////////////////////////////////////////////////
  // apply_t: the operator() is only ever reached through the metafunction
  // indirection (mcall1<CvRef, tuple<Ts...>>), never through the bare name
  // `tuple<>` directly -- this is the shape that matters.
  export struct apply_t
  {
    template <class CvRef>
    struct impl
    {
      template <class... Ts>
      using tuple_t = mcall1<CvRef, tuple<Ts...>>;

      template <class... Ts, class... Us>
      auto operator()(auto&& fn, tuple_t<Ts...>&& /*tupl*/, Us&&... us) const
      {
        return static_cast<decltype(fn)&&>(fn)(static_cast<Us&&>(us)...);
      }
    };

    template <class Tuple>
    using impl_t = impl<copy_cvref_fn<Tuple>>;

    template <class Fn, class Tuple, class... Us>
    constexpr auto operator()(Fn&& fn, Tuple&& tupl, Us&&... us) const
      -> decltype(impl_t<Tuple>{}(static_cast<Fn&&>(fn),
                                  static_cast<Tuple&&>(tupl),
                                  static_cast<Us&&>(us)...))
    {
      return impl_t<Tuple>{}(static_cast<Fn&&>(fn),
                             static_cast<Tuple&&>(tupl),
                             static_cast<Us&&>(us)...);
    }
  };

  export inline constexpr apply_t apply{};

  export struct thing
  {
    template <class Fn>
    constexpr auto run(Fn&& fn)
    {
      tuple<> empty_tuple;
      int     value = 42;
      return apply(static_cast<Fn&&>(fn), empty_tuple, value);
    }
  };

}  // namespace ns
