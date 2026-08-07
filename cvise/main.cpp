import foo;

// Importer-defined, so naming these in visit<> below forces a fresh
// instantiation of the module's visit<> rather than reusing seed()'s.
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
  // Already instantiated in the purview by seed(); nothing formed afresh here.
  bar::visit<bar::module_fn, bar::working_holder<bar::box<>>>({});  // expect pass
  bar::visit<bar::module_fn, bar::broken_holder<bar::box<>>>({});   // expect pass

  // Fresh instantiation, seeded in the purview, via __type_pack_element.
  bar::visit<importer_fn, bar::working_holder<bar::box<>>>({});  // expect pass

  // Fresh instantiation, via pack indexing, over a holder specialization that
  // seed() never forms in the purview: nothing to merge against, so this is
  // accepted.
  bar::visit<importer_fn2, bar::broken_holder<bar::box<double>>>({});  // expect pass

  // Fresh instantiation, via pack indexing, over a holder specialization that
  // seed() *does* form in the purview.  Rejected with a conversion failure from
  // bar::box<> to bar::box<>.
  bar::visit<importer_fn, bar::broken_holder<bar::box<>>>({});  // BROKEN
}
