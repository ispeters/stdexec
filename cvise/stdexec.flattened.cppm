module;
#include <cstdio>
export module stdexec;
#define STDEXEC_IN_MODULE_PURVIEW
#define INCLUDE_STDEXEC___DETAIL___CONFIG_HPP
#define STDEXEC_USE_MODULES() 1
#define STDEXEC_PP_WHEN(...)
import std;
#define STDEXEC_CLANG() 1
#define STDEXEC_EDG() 0
#define STDEXEC_GCC() 0
#define STDEXEC_MSVC() 0
#define STDEXEC
#define STDEXEC_ATTRIBUTE(...)
#define STDEXEC_PRAGMA_PUSH()
#define STDEXEC_PRAGMA_POP()
#define STDEXEC_PRAGMA_IGNORE_GNU(...)
#define STDEXEC_PRAGMA_IGNORE_EDG(...)
#define STDEXEC_HAS_BUILTIN
#define STDEXEC_IS_BASE_OF(...) __is_base_of(__VA_ARGS__)
#define STDEXEC_IS_CONVERTIBLE_TO(...) __is_convertible_to(__VA_ARGS__)
#define STDEXEC_IS_SAME(...) __same_as_v<__VA_ARGS__>
#define STDEXEC_IS_CONSTRUCTIBLE(...) std::is_constructible_v<__VA_ARGS__>
#define STDEXEC_IS_NOTHROW_CONSTRUCTIBLE(...)                                  \
  std::is_nothrow_constructible_v<__VA_ARGS__>
#define STDEXEC_IS_NOTHROW_ASSIGNABLE(...)                                     \
  std::is_nothrow_assignable_v<__VA_ARGS__>
#define STDEXEC_IS_ASSIGNABLE(...) std::is_assignable_v<__VA_ARGS__>
template <class, class> bool __same_as_v;
template <class _Ap> constexpr bool __same_as_v<_Ap, _Ap> = true;
#define STDEXEC_NO_STDCPP_PACK_INDEXING() 0
#define STDEXEC_NO_STDCPP_TYPEID() 0
#define STDEXEC_NO_STDCPP_CONCEPTS_HEADER() 1
#define STDEXEC_MODULE_EXPORT_META
#define STDEXEC_MODULE_EXPORT_AUTHORING
namespace stdexec {
export template <class...> struct my_tuple {};
} // namespace stdexec
#include "__meta.hpp"
namespace stdexec {
export int __no_init;
template <class...> using __call_result_t = decltype(0);
struct __visit_t {
  template <class _Fn, class _Variant>
  void operator()(_Fn, _Variant __var) noexcept(
      noexcept(__var.__visit(_Fn(), __declval<_Variant>())));
};
export __visit_t __visit;
namespace __var {
template <size_t _Ny, class _Variant>
using __variant_alternative_t = __copy_cvref_t<
    _Variant, typename std::remove_reference_t<_Variant>::template __at_t<_Ny>>;
template <size_t _Ny, class _Variant>
auto __get(_Variant) -> __variant_alternative_t<_Ny, _Variant>;
template <size_t _Ny, class, class _Fn, class _Self>
void __visit_alt(_Self __self) {
  _Fn __fn;
  __fn(__get<_Ny>(__self));
}
template <auto, class...> class __variant;
template <size_t... _Is, __indices<_Is...> _Idx, class... _Ts>
struct __variant<_Idx, _Ts...> {
  template <size_t _Ny> using __at_t = __m_at_c<_Ny, _Ts...>;
  __variant(int);
  template <class _Fn> auto __emplace_from(_Fn) -> _Fn;
  template <class _Fn, class _Self> auto __visit(_Fn, _Self) {
    __visit_alt<0, __call_result_t<>, _Fn, _Self>;
  }
};
template <class... _Ts>
using __variant_base_t = __variant<__indices_for<>{}, _Ts...>;
} // namespace __var
export template <class... _Ts>
struct __variant : __var::__variant_base_t<_Ts...> {};
void __seed() {
  __variant<my_tuple<>> __v{__no_init};
  __visit([](auto) {}, __v);
}
} // namespace stdexec
