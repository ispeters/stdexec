import stdexec;

// CONTROL: in the importer's ordinary context my_tuple<> is complete, and
// this passes. Same type, same TU, same specialization as below.
static_assert(stdexec::my_tuple<>::size == 0);

int main() {
  stdexec::__variant<stdexec::my_tuple<>> __args{stdexec::__no_init};

  // ...but reached through __visit_alt -- a template defined in the module,
  // instantiated here -- the same my_tuple<> is INCOMPLETE. The module's
  // __seed() having already instantiated __visit_alt over this exact
  // specialization is required; seeding a different one, or not seeding at
  // all, both compile cleanly.
  __args.__visit(
      [](auto __t) -> void { static_assert(decltype(__t)::size == 0); },
      __args);
}
