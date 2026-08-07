module;
#include <cstdio>
export module stdexec;
#define STDEXEC_IN_MODULE_PURVIEW
#define INCLUDE_STDEXEC___DETAIL___CONFIG_HPP
#define STDEXEC_USE_MODULES() 1
import std;
#define STDEXEC_CLANG() 1
#define STDEXEC_MSVC() 0
#define STDEXEC
#define STDEXEC_PRAGMA_PUSH()
#define STDEXEC_PRAGMA_POP()
#define STDEXEC_HAS_BUILTIN
#define STDEXEC_IS_SAME(...) __same_as_v<__VA_ARGS__>
template <class, class> bool __same_as_v;
#define STDEXEC_MODULE_EXPORT_META
#define STDEXEC_MODULE_EXPORT_AUTHORING
namespace stdexec {
export template <class...> struct my_tuple {};
} // namespace stdexec
/*
 * Copyright (c) 2022 NVIDIA Corporation
 *
 * Licensed under the Apache License Version 2.0 with LLVM Exceptions
 * (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 *   https://llvm.org/LICENSE.txt
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef INCLUDE_STDEXEC___DETAIL___TYPE_TRAITS_HPP
#define INCLUDE_STDEXEC___DETAIL___TYPE_TRAITS_HPP

#include "__config.hpp"

#if STDEXEC_USE_MODULES() && !defined(STDEXEC_IN_MODULE_PURVIEW)

import stdexec;

#else

#  if !STDEXEC_USE_MODULES()
#    include <exception>    // IWYU pragma: keep for std::terminate
#    include <type_traits>  // IWYU pragma: export
#    include <utility>      // IWYU pragma: keep
#  endif

#  include "__prologue.hpp"

namespace STDEXEC
{

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // A very simple std::declval replacement that doesn't handle void
  STDEXEC_MODULE_EXPORT_AUTHORING
  template <class _Tp, bool _Noexcept = true>
  using __declfn_t = auto (*)() noexcept(_Noexcept) -> _Tp;

  STDEXEC_MODULE_EXPORT_AUTHORING
  template <class _Tp, class...>
  extern __declfn_t<_Tp &&> __declval;

  template <class... _NoneSuch>
  extern __declfn_t<void> __declval<void, _NoneSuch...>;

#  if STDEXEC_MSVC()
  template <class _Tp, bool _Noexcept = true>
  _Tp __declfn_() noexcept(_Noexcept)
  {
    STDEXEC_ASSERT(false && +"__declfn() should never be called" == nullptr);
    STDEXEC_TERMINATE();
  }
  template <class _Tp, bool _Noexcept = true>
  inline constexpr __declfn_t<_Tp, _Noexcept> __declfn() noexcept
  {
    return &__declfn_<_Tp, _Noexcept>;
  }
#  else
  template <class _Tp, bool _Noexcept = true>
  using __declfn = __declfn_t<_Tp, _Noexcept>;
#  endif

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // __decay_t: An efficient implementation for std::decay
#  if STDEXEC_HAS_BUILTIN(__decay) && (!STDEXEC_CLANG() || STDEXEC_CLANG_VERSION >= 2100)
  namespace __tt
  {
    template <class>
    struct __wrap;

    template <bool>
    struct __decay_
    {
      template <class _Ty>
      using __f = __decay(_Ty);
    };
  }  // namespace __tt

  STDEXEC_MODULE_EXPORT_AUTHORING
  template <class _Ty>
  using __decay_t = __tt::__decay_<bool(sizeof(__declfn_t<_Ty>))>::template __f<_Ty>;
#  else
  STDEXEC_MODULE_EXPORT_AUTHORING
  template <class _Ty>
  using __decay_t = std::decay_t<_Ty>;
#  endif

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // __copy_cvref_t: For copying cvref from one type to another
  STDEXEC_MODULE_EXPORT_AUTHORING
  struct __cp
  {
    template <class _Tp>
    using __f = _Tp;
  };

  STDEXEC_MODULE_EXPORT_AUTHORING
  struct __cpc
  {
    template <class _Tp>
    using __f = _Tp const;
  };

  STDEXEC_MODULE_EXPORT_AUTHORING
  struct __cplr
  {
    template <class _Tp>
    using __f = _Tp &;
  };

  STDEXEC_MODULE_EXPORT_AUTHORING
  struct __cprr
  {
    template <class _Tp>
    using __f = _Tp &&;
  };

  STDEXEC_MODULE_EXPORT_AUTHORING
  struct __cpclr
  {
    template <class _Tp>
    using __f = _Tp const &;
  };

  STDEXEC_MODULE_EXPORT_AUTHORING
  struct __cpcrr
  {
    template <class _Tp>
    using __f = _Tp const &&;
  };

  template <class>
  extern __cp __cpcvr;
  template <class _Tp>
  extern __cpc __cpcvr<_Tp const>;
  template <class _Tp>
  extern __cplr __cpcvr<_Tp &>;
  template <class _Tp>
  extern __cprr __cpcvr<_Tp &&>;
  template <class _Tp>
  extern __cpclr __cpcvr<_Tp const &>;
  template <class _Tp>
  extern __cpcrr __cpcvr<_Tp const &&>;
  template <class _Tp>
  using __copy_cvref_fn = decltype(__cpcvr<_Tp>);

  STDEXEC_MODULE_EXPORT_AUTHORING
  template <class _From, class _To>
  using __copy_cvref_t = __copy_cvref_fn<_From>::template __f<_To>;

  template <class>
  inline constexpr bool __is_const_ = false;
  template <class _Up>
  inline constexpr bool __is_const_<_Up const> = true;

  namespace __tt
  {
    template <class _Ty>
    constexpr auto __remove_rvalue_reference_fn(_Ty &&) -> _Ty;
  }  // namespace __tt

  template <class _Ty>
  using __remove_rvalue_reference_t = decltype(__tt::__remove_rvalue_reference_fn(
    __declval<_Ty>()));

  // Implemented as a class instead of a free function
  // because of a bizarre nvc++ compiler bug:
  struct __cref_fn
  {
    template <class _Ty>
    constexpr auto operator()(_Ty const &) -> _Ty const &;
  };
  template <class _Ty>
  using __cref_t = decltype(__cref_fn{}(__declval<_Ty>()));

  // Because of nvc++ nvbugs#4679848, we can't make __mbool a simple alias for __mconstant,
  // and because of nvc++ nvbugs#4668709 it can't be a simple alias for std::bool_constant,
  // either. :-(
  // template <bool _Bp>
  // using __mbool = __mconstant<_Bp>;

  STDEXEC_MODULE_EXPORT_META
  template <bool _Bp>
  struct __mbool : std::bool_constant<_Bp>
  {};

  using __mtrue  = __mbool<true>;
  using __mfalse = __mbool<false>;

}  // namespace STDEXEC

#  include "__epilogue.hpp"
#endif  // STDEXEC_USE_MODULES() || defined(STDEXEC_IN_MODULE_PURVIEW)
#endif  // INCLUDE_STDEXEC___DETAIL___TYPE_TRAITS_HPP
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
