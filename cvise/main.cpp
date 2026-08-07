import foo;

struct ifn
{
  // importer-defined -> fresh visit<>
  void operator()(bar::box<>) {}

  // importer-defined with no similar definition in the module
  void operator()(bar::box<int>) {}
};

int main()
{
  bar::visit<bar::mfn, bar::working_holder<bar::box<>>>({});  // control: expect pass
  bar::visit<bar::mfn, bar::broken_holder<bar::box<>>>({});   // control: expect pass
  bar::visit<ifn, bar::working_holder<bar::box<>>>({});       // expect pass
  bar::visit<ifn, bar::broken_holder<bar::box<int>>>({});     // expect pass
  bar::visit<ifn, bar::broken_holder<bar::box<>>>({});        // expect broken
}
