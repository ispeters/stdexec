import foo;

struct importer_fn
{
  // importer-defined -> forces a fresh instantiation of visit<>
  void operator()(bar::box<>) {}
  void operator()(bar::box<int>) {}
};

int main()
{
  // Controls: these name specializations already instantiated in the purview,
  // so nothing is instantiated afresh here.
  bar::visit<bar::module_fn, bar::working_holder<bar::box<>>>({});  // expect pass
  bar::visit<bar::module_fn, bar::broken_holder<bar::box<>>>({});   // expect pass

  // Fresh instantiation via __type_pack_element.
  bar::visit<importer_fn, bar::working_holder<bar::box<>>>({});  // expect pass

  // Fresh instantiation via pack indexing over a non-empty specialization.
  // Now that seed() forms this in the purview too, this line isolates the
  // question "does the empty pack matter?" -- see the note below.
  bar::visit<importer_fn, bar::broken_holder<bar::box<int>>>({});

  // Fresh instantiation via pack indexing over an empty specialization.
  bar::visit<importer_fn, bar::broken_holder<bar::box<>>>({});  // BROKEN
}
