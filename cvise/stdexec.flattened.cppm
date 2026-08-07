module;
#include <cstdio>
export module stdexec;
import std;
#define STDEXEC_IS_SAME(...) __same_as_v<__VA_ARGS__>
template <class, class> bool __same_as_v;
#define STDEXEC_MODULE_EXPORT_AUTHORING
namespace stdexec {
export template <class...> struct my_tuple {};
STDEXEC_MODULE_EXPORT_AUTHORING template <class _Tp, bool _Noexcept = true>
using __declfn_t = auto() noexcept -> _Tp;
STDEXEC_MODULE_EXPORT_AUTHORING template <class _Tp, class...>
extern __declfn_t<_Tp &&> __declval;
STDEXEC_MODULE_EXPORT_AUTHORING struct __cp {
  template <class _Tp> using __f = _Tp;
};
template <class> extern __cp __cpcvr;
template <class _Tp> using __copy_cvref_fn = decltype(__cpcvr<_Tp>);
STDEXEC_MODULE_EXPORT_AUTHORING template <class _From, class _To>
using __copy_cvref_t = __copy_cvref_fn<_From>::template __f<_To>;
} // namespace stdexec
template <size_t _Np> using __msize_t = std::integral_constant<size_t, _Np>;
template <size_t...> struct __iota;
template <size_t... _Is> using __indices = __iota<_Is...> *;
namespace __pack {
template <class _Ty, _Ty...> struct __idx;
template <class> int __mkidx;
template <size_t... _Is> __indices<> __mkidx<__idx<size_t, _Is...>>;
} // namespace __pack
template <size_t _Np>
using __make_indices =
    decltype(__pack::__mkidx<__make_integer_seq<__pack::__idx, size_t, _Np>>);
template <class... _Ts> using __indices_for = __make_indices<sizeof...(_Ts)>;
using __msuccess = int;
template <class... _Args>
concept _Ok = (STDEXEC_IS_SAME(_Args, __msuccess) && ...);
template <int> struct __i {
  template <class _Fn, class... _Args> using __f = _Fn::template __f<_Args...>;
};
template <class _Fn, class... _Args>
using __minvoke = __i<_Ok<>>::__f<_Fn, _Args...>;
template <int>

struct __m_at_ {
  template <class _Np, class... _Ts> using __f = _Ts...[_Np::value];
};
template <size_t _Np, class... _Ts>
using __m_at_c = __minvoke<__m_at_<0>, __msize_t<_Np>, _Ts...>;
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
