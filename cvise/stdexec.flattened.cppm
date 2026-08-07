module;
#include <cstdarg>
#include <cstdio>
export module stdexec;
#define STDEXEC_IN_MODULE_PURVIEW
#define INCLUDE_STDEXEC___DETAIL___CONFIG_HPP
#define STDEXEC_USE_MODULES() 1
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
#ifndef INCLUDE_STDEXEC___DETAIL___PREPROCESSOR_HPP
#define INCLUDE_STDEXEC___DETAIL___PREPROCESSOR_HPP

#define STDEXEC_PP_STRINGIZE_I(...) #__VA_ARGS__
#define STDEXEC_PP_STRINGIZE(...)   STDEXEC_PP_STRINGIZE_I(__VA_ARGS__)

#define STDEXEC_PP_LPAREN() (
#define STDEXEC_PP_RPAREN() )
#define STDEXEC_PP_PARENS() ()
#define STDEXEC_PP_COMMA()  ,

#define STDEXEC_PP_CAT_I(_XP, ...)   _XP##__VA_ARGS__
#define STDEXEC_PP_CAT(_XP, ...)     STDEXEC_PP_CAT_I(_XP, __VA_ARGS__)

#define STDEXEC_PP_EXPAND(...)       __VA_ARGS__
#define STDEXEC_PP_EVAL(_MACRO, ...) _MACRO(__VA_ARGS__)
#define STDEXEC_PP_EAT(...)

#define STDEXEC_PP_IS_EMPTY_I(_BIT, ...) _BIT
#define STDEXEC_PP_IS_EMPTY(...)         STDEXEC_PP_IS_EMPTY_I(__VA_OPT__(0, ) 1)

#define STDEXEC_PP_IIF_0(_YP, ...)       __VA_ARGS__
#define STDEXEC_PP_IIF_1(_YP, ...)       _YP
#define STDEXEC_PP_IIF_EVAL(_MACRO, ...) _MACRO(__VA_ARGS__)
#define STDEXEC_PP_IIF_CAT(_XP, ...)     _XP##__VA_ARGS__
#define STDEXEC_PP_IIF(_XP, _YP, ...)                                                              \
  STDEXEC_PP_IIF_EVAL(STDEXEC_PP_IIF_CAT(STDEXEC_PP_IIF_, _XP), _YP, __VA_ARGS__)

#define STDEXEC_PP_COMPL_0             1 // NOLINT(modernize-macro-to-enum)
#define STDEXEC_PP_COMPL_1             0 // NOLINT(modernize-macro-to-enum)
#define STDEXEC_PP_COMPL_CAT(_XP, ...) _XP##__VA_ARGS__
#define STDEXEC_PP_COMPL(_BIT)         STDEXEC_PP_COMPL_CAT(STDEXEC_PP_COMPL_, _BIT)

#define STDEXEC_PP_COUNT(...)                                                                      \
  STDEXEC_PP_EXPAND(STDEXEC_PP_COUNT_I(__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1))
#define STDEXEC_PP_COUNT_I(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _NP, ...) _NP

#define STDEXEC_PP_FRONT_I(_A1, ...) _A1
#define STDEXEC_PP_FRONT(...)        __VA_OPT__(STDEXEC_PP_FRONT_I(__VA_ARGS__))

// Used to check various properties of arguments
#define STDEXEC_PP_CHECK_EXPAND(...)      __VA_ARGS__
#define STDEXEC_PP_CHECK_I(_XP, _NP, ...) _NP
#define STDEXEC_PP_CHECK(...)             STDEXEC_PP_CHECK_EXPAND(STDEXEC_PP_CHECK_I(__VA_ARGS__, 0, ))
#define STDEXEC_PP_PROBE_I(_XP, _NP, ...) _XP, _NP,
#define STDEXEC_PP_PROBE(...)             STDEXEC_PP_PROBE_I(__VA_ARGS__, 1)

// PP switch statement
#define STDEXEC_PP_CASE_LABEL_(_PREFIX, ...) \
  STDEXEC_PP_EVAL(STDEXEC_PP_CHECK, \
                  STDEXEC_PP_CAT(_PREFIX##_SWITCH_, STDEXEC_PP_FRONT(__VA_ARGS__)), \
                  DEFAULT(STDEXEC_PP_FRONT(__VA_ARGS__)), )
#define STDEXEC_PP_CASE(_ARG) STDEXEC_PP_PROBE(~, _ARG)
#define STDEXEC_PP_SWITCH(_PREFIX, ...) \
  STDEXEC_PP_CAT(_PREFIX##_CASE_, STDEXEC_PP_CASE_LABEL_(_PREFIX, __VA_ARGS__))

// Boolean logic
#define STDEXEC_PP_NOT(_XP)          STDEXEC_PP_CHECK(STDEXEC_PP_CAT(STDEXEC_PP_NOT_, _XP))
#define STDEXEC_PP_NOT_0             STDEXEC_PP_PROBE(~, 1)

#define STDEXEC_PP_BOOL(_XP)         STDEXEC_PP_COMPL(STDEXEC_PP_NOT(_XP))
#define STDEXEC_PP_IF(_XP, _YP, ...) STDEXEC_PP_IIF(STDEXEC_PP_BOOL(_XP), _YP, __VA_ARGS__)

#define STDEXEC_PP_WHEN(_XP, ...)    STDEXEC_PP_IF(_XP, STDEXEC_PP_EXPAND, STDEXEC_PP_EAT)(__VA_ARGS__)

////////////////////////////////////////////////////////////////////////////////
// STDEXEC_PP_FOR_EACH
//   Inspired by "Recursive macros with C++20 __VA_OPT__", by David Mazières
//   https://www.scs.stanford.edu/~dm/blog/va-opt.html
#define STDEXEC_PP_EXPAND_R3(...)                                                                  \
  STDEXEC_PP_EXPAND(STDEXEC_PP_EXPAND(STDEXEC_PP_EXPAND(STDEXEC_PP_EXPAND(__VA_ARGS__))))
#define STDEXEC_PP_EXPAND_R2(...)                                                                  \
  STDEXEC_PP_EXPAND_R3(                                                                            \
    STDEXEC_PP_EXPAND_R3(STDEXEC_PP_EXPAND_R3(STDEXEC_PP_EXPAND_R3(__VA_ARGS__))))
#define STDEXEC_PP_EXPAND_R1(...)                                                                  \
  STDEXEC_PP_EXPAND_R2(                                                                            \
    STDEXEC_PP_EXPAND_R2(STDEXEC_PP_EXPAND_R2(STDEXEC_PP_EXPAND_R2(__VA_ARGS__))))
#define STDEXEC_PP_EXPAND_R(...)                                                                   \
  STDEXEC_PP_EXPAND_R1(                                                                            \
    STDEXEC_PP_EXPAND_R1(STDEXEC_PP_EXPAND_R1(STDEXEC_PP_EXPAND_R1(__VA_ARGS__))))

#define STDEXEC_PP_FOR_EACH_AGAIN() STDEXEC_PP_FOR_EACH_HELPER
#define STDEXEC_PP_FOR_EACH_HELPER(_MACRO, _A1, ...)                                               \
  _MACRO(_A1) __VA_OPT__(STDEXEC_PP_FOR_EACH_AGAIN STDEXEC_PP_PARENS()(_MACRO, __VA_ARGS__)) /**/
#define STDEXEC_PP_FOR_EACH(_MACRO, ...)                                                           \
  __VA_OPT__(STDEXEC_PP_EXPAND_R(STDEXEC_PP_FOR_EACH_HELPER(_MACRO, __VA_ARGS__)))

////////////////////////////////////////////////////////////////////////////////////////////////////

#define STDEXEC_PP_BACK_AGAIN()      STDEXEC_PP_BACK_I
#define STDEXEC_PP_BACK_I(_A1, ...)                                                                \
  STDEXEC_PP_FRONT(__VA_OPT__(, ) _A1, )                                                           \
  __VA_OPT__(STDEXEC_PP_BACK_AGAIN STDEXEC_PP_PARENS()(__VA_ARGS__))
#define STDEXEC_PP_BACK(...)                 __VA_OPT__(STDEXEC_PP_EXPAND_R(STDEXEC_PP_BACK_I(__VA_ARGS__)))

#define STDEXEC_PP_TAIL(_IGN, ...)           __VA_ARGS__

#define STDEXEC_PP_REPEAT_I(_N, _MACRO, ...) STDEXEC_PP_REPEAT_##_N(_MACRO, __VA_ARGS__)
#define STDEXEC_PP_REPEAT_0(_MACRO, ...)
#define STDEXEC_PP_REPEAT_1(_MACRO, ...) _MACRO(0 __VA_OPT__(, ) __VA_ARGS__)
#define STDEXEC_PP_REPEAT_2(_MACRO, ...)                                                           \
  STDEXEC_PP_REPEAT_1(_MACRO, __VA_ARGS__) _MACRO(1 __VA_OPT__(, ) __VA_ARGS__)
#define STDEXEC_PP_REPEAT_3(_MACRO, ...)                                                           \
  STDEXEC_PP_REPEAT_2(_MACRO, __VA_ARGS__) _MACRO(2 __VA_OPT__(, ) __VA_ARGS__)
#define STDEXEC_PP_REPEAT_4(_MACRO, ...)                                                           \
  STDEXEC_PP_REPEAT_3(_MACRO, __VA_ARGS__) _MACRO(3 __VA_OPT__(, ) __VA_ARGS__)
#define STDEXEC_PP_REPEAT_5(_MACRO, ...)                                                           \
  STDEXEC_PP_REPEAT_4(_MACRO, __VA_ARGS__) _MACRO(4 __VA_OPT__(, ) __VA_ARGS__)
#define STDEXEC_PP_REPEAT_6(_MACRO, ...)                                                           \
  STDEXEC_PP_REPEAT_5(_MACRO, __VA_ARGS__) _MACRO(5 __VA_OPT__(, ) __VA_ARGS__)
#define STDEXEC_PP_REPEAT_7(_MACRO, ...)                                                           \
  STDEXEC_PP_REPEAT_6(_MACRO, __VA_ARGS__) _MACRO(6 __VA_OPT__(, ) __VA_ARGS__)
#define STDEXEC_PP_REPEAT_8(_MACRO, ...)                                                           \
  STDEXEC_PP_REPEAT_7(_MACRO, __VA_ARGS__) _MACRO(7 __VA_OPT__(, ) __VA_ARGS__)
#define STDEXEC_PP_REPEAT_9(_MACRO, ...)                                                           \
  STDEXEC_PP_REPEAT_8(_MACRO, __VA_ARGS__) _MACRO(8 __VA_OPT__(, ) __VA_ARGS__)
#define STDEXEC_PP_REPEAT_10(_MACRO, ...)                                                          \
  STDEXEC_PP_REPEAT_9(_MACRO, __VA_ARGS__) _MACRO(9 __VA_OPT__(, ) __VA_ARGS__)
#define STDEXEC_PP_REPEAT(_N, _MACRO, ...) STDEXEC_PP_REPEAT_I(_N, _MACRO, __VA_ARGS__)
#endif  // INCLUDE_STDEXEC___DETAIL___PREPROCESSOR_HPP
import std;
#define STDEXEC_CLANG() 1
#define STDEXEC_EDG() 0
#define STDEXEC_GCC() 0
#define STDEXEC_MSVC() 0
#define STDEXEC_HOST_DEVICE_DEDUCTION_GUIDE
#define STDEXEC stdexec
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
#define STDEXEC_REMOVE_REFERENCE(...) std::remove_reference_t<__VA_ARGS__>
template <class, class> bool __same_as_v;
template <class _Ap> constexpr bool __same_as_v<_Ap, _Ap> = true;
#define STDEXEC_UNREACHABLE std::terminate
#define STDEXEC_IMMOVABLE(_XP)
#if !0
#define STDEXEC_NO_STDCPP_PACK_INDEXING() 0
#endif
#define STDEXEC_NO_STDCPP_RTTI() 1
#define STDEXEC_NO_STDCPP_TYPEID() 0
#define STDEXEC_NO_STDCPP_CONCEPTS_HEADER() 1
#define STDEXEC_IF_CONSTEVAL if (std::is_constant_evaluated)
#define STDEXEC_MODULE_EXPORT_META
#define STDEXEC_MODULE_EXPORT_AUTHORING export
namespace stdexec {
export template <class...> struct my_tuple {};
} // namespace stdexec
/*
 * Copyright (c) 2023 NVIDIA Corporation
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
#ifndef INCLUDE_STDEXEC___DETAIL___SCOPE_HPP
#define INCLUDE_STDEXEC___DETAIL___SCOPE_HPP

#include "__config.hpp"

#if STDEXEC_USE_MODULES() && !defined(STDEXEC_IN_MODULE_PURVIEW)

import stdexec;

#else

#  include "__utility.hpp"

#  if !STDEXEC_USE_MODULES()
#    include <type_traits>
#  endif

#  include "__prologue.hpp"

namespace STDEXEC
{
  STDEXEC_MODULE_EXPORT_AUTHORING
  template <class _Fn, class... _Ts>
  struct __scope_guard;

  template <class _Fn>
  struct __scope_guard<_Fn>
  {
    STDEXEC_ATTRIBUTE(no_unique_address) _Fn __fn_;
    STDEXEC_ATTRIBUTE(no_unique_address) __immovable __hidden_ {};
    bool __dismissed_{false};

    constexpr ~__scope_guard()
    {
      if (!__dismissed_)
        static_cast<_Fn&&>(__fn_)();
    }

    constexpr void __dismiss() noexcept
    {
      __dismissed_ = true;
    }
  };

  template <class _Fn, class _T0>
  struct __scope_guard<_Fn, _T0>
  {
    STDEXEC_ATTRIBUTE(no_unique_address) _Fn __fn_;
    STDEXEC_ATTRIBUTE(no_unique_address) _T0 __t0_;
    STDEXEC_ATTRIBUTE(no_unique_address) __immovable __hidden_ {};

    bool __dismissed_{false};

    constexpr void __dismiss() noexcept
    {
      __dismissed_ = true;
    }

    constexpr ~__scope_guard()
    {
      if (!__dismissed_)
        static_cast<_Fn&&>(__fn_)(static_cast<_T0&&>(__t0_));
    }
  };

  template <class _Fn, class _T0, class _T1>
  struct __scope_guard<_Fn, _T0, _T1>
  {
    STDEXEC_ATTRIBUTE(no_unique_address) _Fn __fn_;
    STDEXEC_ATTRIBUTE(no_unique_address) _T0 __t0_;
    STDEXEC_ATTRIBUTE(no_unique_address) _T1 __t1_;
    STDEXEC_ATTRIBUTE(no_unique_address) __immovable __hidden_ {};

    bool __dismissed_{false};

    constexpr void __dismiss() noexcept
    {
      __dismissed_ = true;
    }

    constexpr ~__scope_guard()
    {
      if (!__dismissed_)
        static_cast<_Fn&&>(__fn_)(static_cast<_T0&&>(__t0_), static_cast<_T1&&>(__t1_));
    }
  };

  template <class _Fn, class _T0, class _T1, class _T2>
  struct __scope_guard<_Fn, _T0, _T1, _T2>
  {
    STDEXEC_ATTRIBUTE(no_unique_address) _Fn __fn_;
    STDEXEC_ATTRIBUTE(no_unique_address) _T0 __t0_;
    STDEXEC_ATTRIBUTE(no_unique_address) _T1 __t1_;
    STDEXEC_ATTRIBUTE(no_unique_address) _T2 __t2_;
    STDEXEC_ATTRIBUTE(no_unique_address) __immovable __hidden_ {};

    bool __dismissed_{false};

    constexpr void __dismiss() noexcept
    {
      __dismissed_ = true;
    }

    constexpr ~__scope_guard()
    {
      if (!__dismissed_)
        static_cast<_Fn&&>(
          __fn_)(static_cast<_T0&&>(__t0_), static_cast<_T1&&>(__t1_), static_cast<_T2&&>(__t2_));
    }
  };

  template <class _Fn, class... _Ts>
  STDEXEC_HOST_DEVICE_DEDUCTION_GUIDE
  __scope_guard(_Fn, _Ts...) -> __scope_guard<_Fn, std::unwrap_reference_t<_Ts>...>;
}  // namespace STDEXEC

#  include "__epilogue.hpp"
#endif  // !STDEXEC_USE_MODULES() || defined(STDEXEC_IN_MODULE_PURVIEW)
#endif  // INCLUDE_STDEXEC___DETAIL___SCOPE_HPP
namespace stdexec {
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
  __variant(__no_init_t);
  template <class _Fn> auto __emplace_from(_Fn) -> _Fn;
  template <class _Fn, class _Self> auto __visit(_Fn, _Self) {
    using __result_t = __call_result_t<_Fn, __copy_cvref_t<_Self, __at_t<0>>>;
    __visit_alt<0, __result_t, _Fn, _Self>;
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
