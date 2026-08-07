// Control: exactly the code in foo.cppm + main.cpp, with all module syntax
// removed and both translation units concatenated.  Expect no diagnostics --
// this is what establishes that the defect is modules-specific rather than a
// general problem with C++26 pack indexing.
namespace bar
{

  template <class... Ts>
  struct box
  {};

  template <class... Ts>
  struct broken_holder
  {
    template <unsigned N>
    using at = Ts...[N];
  };

  template <class... Ts>
  struct working_holder
  {
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

  void seed()
  {
    visit<module_fn, broken_holder<box<>>>({});
    visit<module_fn, working_holder<box<>>>({});
  }
}  // namespace bar

struct importer_fn
{
  void operator()(bar::box<>) {}
};

struct importer_fn2
{
  void operator()(bar::box<double>) {}
};

int main()
{
  bar::visit<bar::module_fn, bar::working_holder<bar::box<>>>({});
  bar::visit<bar::module_fn, bar::broken_holder<bar::box<>>>({});
  bar::visit<importer_fn, bar::working_holder<bar::box<>>>({});
  bar::visit<importer_fn2, bar::broken_holder<bar::box<double>>>({});
  bar::visit<importer_fn, bar::broken_holder<bar::box<>>>({});
}
