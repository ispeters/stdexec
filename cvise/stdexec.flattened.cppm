module;
#include <cstddef>
export module stdexec;

namespace stdexec {

  export template <class... _Ts>
  struct my_tuple {
    static constexpr int size = sizeof...(_Ts);
  };

  export int __no_init;

  namespace __var {
    template <std::size_t _Ny, class _Variant>
    using __variant_alternative_t = typename _Variant::template __at_t<_Ny>;

    template <std::size_t _Ny, class _Variant>
    auto __get(_Variant) -> __variant_alternative_t<_Ny, _Variant>;

    template <std::size_t _Ny, class, class _Fn, class _Self>
    void __visit_alt(_Self __self) {
      _Fn __fn;
      __fn(__get<_Ny>(__self));
    }
  } // namespace __var

  export template <class... _Ts>
  struct __variant {
    template <std::size_t _Ny>
    using __at_t = _Ts...[_Ny];

    __variant(int);

    template <class _Fn, class _Self>
    auto __visit(_Fn, _Self) {
      (void) &__var::__visit_alt<0, int, _Fn, _Self>;
    }
  };

  // THE SEED: instantiates __visit_alt over my_tuple<> inside the purview.
  void __seed() {
    __variant<my_tuple<>> __v{__no_init};
    __v.__visit([](auto) {}, __v);
  }
} // namespace stdexec
