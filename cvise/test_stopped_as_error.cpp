import stdexec;
import std;

int main() {
  // Mirrors the module's __seed() exactly, but in the IMPORTER: __no_init,
  // __emplace_from, __visit, over the SAME specialization (my_tuple<>) the
  // module already instantiated __visit_alt over. This is the configuration
  // confirmed, via the ladder investigation, to reproduce the bug:
  //   - a plain __variant<my_tuple<>> with no module-side seed: clean.
  //   - a module-side seed over a DIFFERENT specialization (my_tuple<double>
  //     while this uses my_tuple<int>, or vice versa): clean.
  //   - a module-side seed over this EXACT specialization: fails, with
  //     my_tuple<> reported as having no members ("cannot be used prior to
  //     '::'") despite being findable and same_as-identical to the module's
  //     own my_tuple<> -- an incompleteness diagnosis, not an identity one.
  stdexec::__variant<stdexec::my_tuple<>> __args{stdexec::__no_init};
  __args.__emplace_from([] { return stdexec::my_tuple<>{}; });
  stdexec::__visit(
      [](auto &&__t) -> void {
        // THE TRIGGER. static_assert rather than just accessing __t so the
        // failure is a compile error with useful diagnostic text, not a
        // silently-ignored unused reference.
        static_assert(std::remove_reference_t<decltype(__t)>::size == 0);
      },
      __args);
}
