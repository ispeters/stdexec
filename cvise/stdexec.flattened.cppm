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
  // ORDERING PROBE: the "no known conversion from X to X" diagnostic is a
  // directly observed identity split, not an inferred one -- overload
  // resolution rejected an argument against a parameter with the identical
  // qualified spelling. That's consistent with a known shape of Clang
  // modules bug: two independent implicit instantiations of the same class
  // template specialization, from different TUs, failing to merge.
  //
  // If it's an ordering/merge bug, forcing the module's own compilation to
  // instantiate just()'s __tuple<> FIRST -- so a canonical specialization
  // already exists in the BMI before the importer creates its own -- may be
  // enough to make the importer's instantiation resolve to the same entity
  // instead of a fresh one. That would be directly usable as a source-level
  // workaround: an internal "priming" call, not a redesign.
  inline void __prime() {
    STDEXEC::just();
  }

  // LADDER STEP 2: a hand-rolled stand-in for __tuple, declared in the module
  // so the seed below can instantiate over it. If the failure reproduces with
  // this in place of __tuple<>, then __tuple is not special -- no __box, no
  // __tupl_base, no __make_indices, no explicit specialisation -- and any
  // class template specialisation will do. That removes __tuple.hpp entirely
  // from the standalone repro.
  export template <class... _Ts>
  struct my_tuple {
    static constexpr int size = sizeof...(_Ts);
  };

  // LADDER STEP 3: two hand-rolled stand-ins for __variant, to isolate whether
  // the FUNCTION-POINTER TABLE is part of the mechanism. stdexec's __visit
  // builds `{&__var::__visit_alt<_Is, __result_t, _Fn, _Self, _Us...>...}`
  // (__variant.hpp:384) -- taking the address of a specialisation whose
  // instantiation is what fails. Flavour A mirrors that; flavour B calls the
  // visitor directly with no address-of and no table. If A reproduces and B
  // does not, the indirection is essential and belongs in the bug report.
  export template <class _Ty>
  struct my_variant_ptr {
    _Ty __value;

    template <int _Ny, class _Result, class _Fn, class _Self>
    static constexpr auto visit_alt(_Fn &&__fn, _Self &&__self) -> _Result {
      return static_cast<_Fn &&>(__fn)(__self.__value);
    }

    template <class _Fn, class _Self>
    static constexpr void visit(_Fn &&__fn, _Self &&__self) {
      using __fnptr_t = void (*)(_Fn &&, _Self &&);
      static constexpr __fnptr_t __table[] = {
          &my_variant_ptr::template visit_alt<0, void, _Fn, _Self>};
      __table[0](static_cast<_Fn &&>(__fn), static_cast<_Self &&>(__self));
    }
  };

  export template <class _Ty>
  struct my_variant_direct {
    _Ty __value;

    template <class _Fn, class _Self>
    static constexpr void visit(_Fn &&__fn, _Self &&__self) {
      static_cast<_Fn &&>(__fn)(__self.__value);
    }
  };

  inline void __seed_my_variant() {
    my_variant_ptr<my_tuple<>> __a{};
    my_variant_ptr<my_tuple<>>::visit([](auto &) noexcept {}, __a);
    my_variant_direct<my_tuple<>> __b{};
    my_variant_direct<my_tuple<>>::visit([](auto &) noexcept {}, __b);
  }

  // ---- WORKAROUND CANDIDATE: EXPLICIT INSTANTIATION DEFINITIONS ----------
  // The seed must name the EXACT specialization the importer uses (my_tuple<int>
  // seeded + my_tuple<int> used => fails; my_tuple<double> seeded => clean).
  // So a module-side IMPLICIT instantiation is what leaves the specialization
  // declared-but-not-defined in the BMI. An explicit instantiation definition
  // forces the definition to be emitted, and may serialize it properly.
  //
  // Comment these out individually to bisect. If the my_tuple ones clear the
  // step-2 errors AND the __tuple one clears the original stdexec failure,
  // this is a shippable workaround: a handful of explicit instantiations in
  // the module interface, no restructuring of task_scheduler required.
  template struct my_tuple<>;
  template struct my_tuple<int>;

  namespace __tup {
    template struct __tuple<>;
  } // namespace __tup

  // Module-side seed over my_tuple, mirroring __seed's shape exactly.
  inline void __seed_my_tuple() {
    __variant<my_tuple<>> __v{__no_init};
    __visit([](auto &...) noexcept {}, __v);
    __variant<my_tuple<int>> __w{__no_init};
    __visit([](auto &...) noexcept {}, __w);
  }

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

  // Probe 4: unlike the three above, this routes __tuple<> through the SAME
  // code path that actually fails -- just_t::operator() -> __make_sexpr ->
  // __sexpr<__desc<just_t, __tuple<>>{}>. __sexpr's single template parameter
  // is `auto _DescriptorFn`: a class-type NON-TYPE template parameter. Two
  // specializations of __sexpr are the same specialization only if Clang
  // considers the two __desc<...>{} VALUES structurally equal, which is a
  // narrower and newer piece of machinery than plain class-template-argument
  // matching. If the split lives there rather than in __tuple<> per se, THIS
  // is the probe that should fail where the first three didn't.
  export using __probe_just_result_t = decltype(STDEXEC::just());
} // namespace STDEXEC
