export module foo;
export namespace bar
{

  template <class... Ts>
  struct box
  {};

  template <class... Ts>
  struct broken_holder
  {
    // using C++26 type pack indexing breaks
    template <unsigned N>
    using at = Ts...[N];
  };

  template <class... Ts>
  struct working_holder
  {
    // but using __type_pack_element works
    template <unsigned N>
    using at = __type_pack_element<N, Ts...>;
  };

  template <class H>
  auto get(H) -> typename H::template at<0>
  {
    return {};
  }

  template <class Fn, class H>
  void visit(H h)
  {
    Fn{}(get(h));
  }

  struct module_fn
  {
    void operator()(box<>) {}
  };
}  // namespace bar

namespace bar
{
  void seed()
  {
    // Form at<0> in the module purview.  Note that broken_holder<box<double>>
    // is deliberately *not* named here; see main.cpp.
    visit<module_fn, broken_holder<box<>>>({});
    visit<module_fn, working_holder<box<>>>({});
  }
}  // namespace bar
