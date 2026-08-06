module;
#include <cassert>
#include <cstdarg>
#include <cstdio>
export module stdexec;
#define STDEXEC_IN_MODULE_PURVIEW
#include "__detail/__let.hpp"
#include "__detail/__read_env.hpp"
#include "__just.hpp"

// ---------------------------------------------------------------------------
// SEEDING PROBE -- replaces ~140 lines of task_scheduler machinery.
//
// Hypothesis: that machinery was load-bearing only because it instantiated
// __variant<__tuple<>> inside the module purview, via
//   __just_with_scheduler::get_completion_signatures()
//     -> completion_signatures<set_value_t()>          // EMPTY value pack
//     -> value_types_of_t<..., __decayed_tuple, __uniqued_variant>
//     -> __variant<__tuple<>>
// while the importing TU instantiates the same type as
// __let::__opstate::__args_. Two contexts, same spelling -- a BMI identity
// split. SEED_LEVEL 2 tested that directly and did NOT reproduce, so a bare
// instantiation is not sufficient.
//
// This test instantiates __visit over a __variant<__tuple<>>, as
// __let::__opstate::__start_next does, and testing confirms it repros.
//
// Committing like this and going to bed so I can continue fresh tomorrow.
// ---------------------------------------------------------------------------

namespace STDEXEC {
  inline void __seed() {
    __variant<__tuple<>> __v{__no_init};
    __visit([](auto &...) noexcept {}, __v);
  }

  // ---- IDENTITY PROBES ---------------------------------------------------
  // We have inferred a BMI identity split but never directly observed one.
  // These aliases name the types AS THE MODULE SEES THEM; the importer
  // compares them against the types it forms itself (see the bottom of
  // test_stopped_as_error.cpp). Three probes, because the level-2/3a results
  // suggest the split may not affect all three equally:
  //   1. __tuple<> named directly
  //   2. __variant<__tuple<>> named directly
  //   3. __variant<__tuple<>> produced through the SAME alias/meta chain the
  //      bulk path used -- 3a showed that producing it this way module-side
  //      is not sufficient to seed the bug, but it may still be where the
  //      split shows up.
  export using __probe_tuple_t = __tuple<>;
  export using __probe_variant_t = __variant<__tuple<>>;
  export using __probe_alias_variant_t = __uniqued_variant<__decayed_tuple<>>;
} // namespace STDEXEC
