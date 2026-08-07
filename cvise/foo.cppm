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
  auto get(H) -> typename H::template at<0>;

  template <class Fn, class H>
  void visit(H h)
  {
    Fn{}(get(h));
  }

  struct mfn
  {
    void operator()(box<>) {}
  };
}  // namespace bar

namespace bar
{
  void seed()
  {
    // form both versions of at<N> in the module purview
    visit<mfn, broken_holder<box<>>>({});
    visit<mfn, working_holder<box<>>>({});
  }
}  // namespace bar
