/* Copyright (c) 2026 Ian Petersen
 * Copyright (c) 2026 NVIDIA Corporation
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
#pragma once

#include "__atomic_intrusive_slist.hpp"
#include "__chunked_caching_resource.hpp"
#include "get_frame_allocator.hpp"

#include "../stdexec/__detail/__basic_sender.hpp"
#include "../stdexec/__detail/__env.hpp"
#include "../stdexec/__detail/__get_completion_signatures.hpp"
#include "../stdexec/__detail/__queries.hpp"
#include "../stdexec/__detail/__sender_adaptor_closure.hpp"
#include "../stdexec/__detail/__sender_concepts.hpp"
#include "../stdexec/__detail/__sender_introspection.hpp"
#include "../stdexec/__detail/__senders.hpp"

#include <memory>

namespace experimental::execution
{
  namespace __with_frame_alloc
  {
    using namespace STDEXEC;

    template <class _Receiver, class _Resource>
    struct __opstate
    {
      _Receiver         __rcvr_;
      mutable _Resource __resource_;
    };

    template <class _Resource>
    struct with_frame_allocator_t : sender_adaptor_closure<with_frame_allocator_t<_Resource>>
    {
      [[nodiscard]]
      constexpr auto operator()() const noexcept
      {
        return *this;
      }

      template <sender _Sender>
      constexpr __well_formed_sender auto operator()(_Sender&& __sndr) const
      {
        return __make_sexpr<with_frame_allocator_t<_Resource>>({}, static_cast<_Sender&&>(__sndr));
      }
    };

    template <class _Resource, class... _Env>
    using __make_env_t = __join_env_t<prop<get_frame_allocator_t, _Resource*>, _Env...>;

    template <class _Resource>
    struct __impls : __sexpr_defaults
    {
      static constexpr auto __get_attrs =
        []<class _Child>(__ignore, __ignore, _Child const & __child) noexcept
      {
        return __sync_attrs{__child};
      };

      static constexpr auto __get_env =
        []<class _Receiver>(__ignore, __opstate<_Receiver, _Resource> const & __state) noexcept
        -> __make_env_t<_Resource, __fwd_env_t<env_of_t<_Receiver>>>
      {
        return __env::__join(prop(get_frame_allocator, std::addressof(__state.__resource_)),
                             __fwd_env(STDEXEC::get_env(__state.__rcvr_)));
      };

      static constexpr auto __get_state =
        []<class _Sender, class _Receiver>(_Sender const &,
                                           _Receiver __rcvr) -> __opstate<_Receiver, _Resource>
        requires sender_in<__child_of<_Sender>,
                           __make_env_t<_Resource, __fwd_env_t<env_of_t<_Receiver>>>>
      {
        return __opstate<_Receiver, _Resource>{static_cast<_Receiver&&>(__rcvr)};
      };

      template <class _Self>
        requires(!dependent_sender<__child_of<_Self>>)
      static consteval auto __get_completion_signatures()
      {
        static_assert(__sender_for<_Self, with_frame_allocator_t<_Resource>>);
        return STDEXEC::get_completion_signatures<__child_of<_Self>>();
      }

      template <class _Self, class _Env>
      static consteval auto __get_completion_signatures()
      {
        static_assert(__sender_for<_Self, with_frame_allocator_t<_Resource>>);
        return STDEXEC::get_completion_signatures<__child_of<_Self>,
                                                  __make_env_t<_Resource, __fwd_env_t<_Env>>>();
      }
    };

    template <bool _Synchronized>
    using __default_resource = __chunked_caching_resource<__linked_stack_cache<_Synchronized>>;
  }  // namespace __with_frame_alloc

  using with_frame_allocator_t =
    __with_frame_alloc::with_frame_allocator_t<__with_frame_alloc::__default_resource<true>>;
  inline constexpr with_frame_allocator_t with_frame_allocator{};

  using with_unsynchronized_frame_allocator_t =
    __with_frame_alloc::with_frame_allocator_t<__with_frame_alloc::__default_resource<false>>;
  inline constexpr with_unsynchronized_frame_allocator_t with_unsynchronized_frame_allocator{};
}  // namespace experimental::execution

namespace exec = experimental::execution;

namespace STDEXEC
{
  template <class _Resource>
  struct __sexpr_impl<exec::__with_frame_alloc::with_frame_allocator_t<_Resource>>
    : exec::__with_frame_alloc::__impls<_Resource>
  {};
}  // namespace STDEXEC
