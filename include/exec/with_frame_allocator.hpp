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

#include "get_frame_allocator.hpp"

#include "../stdexec/execution.hpp"

#include <memory>

namespace experimental::execution
{
  namespace __with_frame_alloc
  {
    using namespace STDEXEC;

    struct __unsynchronized_stack
    {
      constexpr bool empty() const noexcept
      {
        return __stack_.empty();
      }

      constexpr void push(void* __p) noexcept
      {
        __stack_.push_front(new (__p) __node());
      }

      constexpr void* pop() noexcept
      {
        return __stack_.pop_front();
      }

      constexpr auto begin() noexcept
      {
        return __stack_.begin();
      }

      constexpr auto end() noexcept
      {
        return __stack_.end();
      }

     private:
      struct __node
      {
        __node* __next_{};
      };

      __intrusive_slist<&__node::__next_> __stack_;
    };

    template <class _Stack>
    concept __free_stack =        //
      std::ranges::range<_Stack>  //
      && requires(_Stack& __stack, _Stack const & __cstack) {
           { __cstack.empty() } -> __same_as<bool>;
           { __stack.pop() } -> __same_as<void*>;
           { __stack.push(static_cast<void*>(nullptr)) };
         };

    static_assert(__free_stack<__unsynchronized_stack>);

    template <__free_stack _FreeStack = __unsynchronized_stack>
    struct __free_stack_recycler : std::pmr::memory_resource
    {
      constexpr __free_stack_recycler() noexcept = default;

      STDEXEC_IMMOVABLE(__free_stack_recycler);

      constexpr ~__free_stack_recycler()
      {
        std::size_t __size = __smallest_allocation;
        for (auto& __stack: __free_stacks_)
        {
          for (void* __allocation: __stack)
          {
            ::operator delete(__allocation, __size);
          }

          __size <<= 1;
        }
      }

      constexpr void* allocate(std::size_t __n, std::size_t)
      {
        // check if there's a free stack that might contain an allocation we can reuse
        if (auto __stack_index = __free_stack_index_of(__n);
            __stack_index < __allocation_size_count)
        {
          // check if that stack has an allocation we can reuse
          if (auto& __free_stack = __free_stacks_[__stack_index]; !__free_stack.empty())
          {
            return __free_stack.pop();
          }
          else
          {
            // allocate more bytes than asked for so that we can put this allocation back
            // into a free stack when it's deallocated
            return ::operator new(__allocation_size_of(__n));
          }
        }

        // __n is bigger than the largest cached allocation so just punt to ::new
        return ::operator new(__n);
      }

      constexpr void deallocate(void* __p, std::size_t __n, std::size_t) noexcept
      {
        if (auto __stack_index = __free_stack_index_of(__n);
            __stack_index < __allocation_size_count)
        {
          // cache the allocation addressed by __p for later use
          STDEXEC_TRY
          {
            __free_stacks_[__stack_index].push(__p);
          }
          STDEXEC_CATCH(...)
          {
            // oops; fall through to the delete, below
            goto do_delete;
          }
        }

do_delete:
        // __p points at an allocation that's too big for us to have cached
        ::operator delete(__p, __n);
      }

     private:
      constexpr void* do_allocate(std::size_t __n, std::size_t __a) final
      {
        return allocate(__n, __a);
      }

      constexpr void do_deallocate(void* __p, std::size_t __n, std::size_t __a) noexcept final
      {
        return deallocate(__p, __n, __a);
      }

      constexpr bool do_is_equal(std::pmr::memory_resource const & __other) const noexcept final
      {
        return static_cast<std::pmr::memory_resource const *>(this) == &__other;
      }

      // the number of bytes in the smallest allocation
      static constexpr std::size_t __smallest_allocation = 64;
      // the number of allocation sizes before falling back to ::new
      static constexpr std::size_t __allocation_size_count = 10;

      static constexpr std::size_t __allocation_size_of(std::size_t __bytes) noexcept
      {
        // round __bytes up to the nearest power of two that is at least
        // __smallest_allocation
        return std::bit_ceil(__bytes | __smallest_allocation);
      }

      static constexpr std::size_t __free_stack_index_of(std::size_t __bytes) noexcept
      {
        // compute the index into __free_stacks_ containing the cached allocations for
        // allocations of size __bytes; returns an index larger than the largest valid
        // index for values of __bytes whose allocations will not be cached
        return std::countr_zero(__allocation_size_of(__bytes))
             - std::countr_zero(__smallest_allocation);
      }

      std::array<_FreeStack, __allocation_size_count> __free_stacks_{};
    };

    template <class _Resource>
    concept __memory_resource = requires(_Resource& __resource) {
      { __resource.allocate(std::size_t{}, std::size_t{}) } -> __same_as<void*>;
      { __resource.deallocate((void*) nullptr, std::size_t{}, std::size_t{}) } -> __same_as<void>;
    };

    template <__memory_resource _Resource>
    struct __resource
    {
      constexpr explicit __resource(_Resource& __rsrc) noexcept
        : __resource_(std::addressof(__rsrc))
      {}

      operator std::pmr::polymorphic_allocator<std::byte>() const noexcept
      {
        return std::pmr::polymorphic_allocator<std::byte>(__resource_);
      }

     protected:
      constexpr void* __allocate(std::size_t __n)
      {
        return __resource_->allocate(__n, alignof(std::max_align_t));
      }

      constexpr void __deallocate(void* __p, std::size_t __n) noexcept
      {
        __resource_->deallocate(__p, __n, alignof(std::max_align_t));
      }

     private:
      _Resource* __resource_;

      friend bool operator==(__resource __lhs, __resource __rhs) noexcept
      {
        return *__lhs.__resource_ == *__rhs.__resource_;
      }
    };

    template <class _Ty, __memory_resource _Resource = __free_stack_recycler<>>
    struct __frame_allocator : __resource<_Resource>
    {
      using value_type = _Ty;
      using pointer    = value_type*;

      using __resource<_Resource>::__resource;

      constexpr pointer allocate(std::size_t __n)
      {
        return static_cast<pointer>(this->__allocate(__n));
      }

      constexpr void deallocate(pointer __p, std::size_t __n) noexcept
      {
        this->__deallocate(__p, __n);
      }
    };

    template <class _Receiver>
    struct __opstate
    {
      _Receiver                       __rcvr_;
      mutable __free_stack_recycler<> __resource_;
    };

    struct with_frame_allocator_t : sender_adaptor_closure<with_frame_allocator_t>
    {
      [[nodiscard]]
      constexpr auto operator()() const noexcept
      {
        return *this;
      }

      template <sender _Sender>
      constexpr __well_formed_sender auto operator()(_Sender&& __sndr) const
      {
        return __make_sexpr<with_frame_allocator_t>({}, static_cast<_Sender&&>(__sndr));
      }
    };

    template <class... _Env>
    using __make_env_t =
      __join_env_t<prop<get_frame_allocator_t, __frame_allocator<std::byte>>, _Env...>;

    struct __impls : __sexpr_defaults
    {
      static constexpr auto __get_attrs =
        []<class _Child>(__ignore, __ignore, _Child const & __child) noexcept
      {
        return __sync_attrs{__child};
      };

      static constexpr auto __get_env =
        []<class _Receiver>(__ignore, __opstate<_Receiver> const & __state) noexcept
        -> __make_env_t<env_of_t<_Receiver>>
      {
        return __env::__join(prop(get_frame_allocator,
                                  __frame_allocator<std::byte>(__state.__resource_)),
                             STDEXEC::get_env(__state.__rcvr_));
      };

      static constexpr auto __get_state =
        []<class _Sender, class _Receiver>(_Sender&& __sndr,
                                           _Receiver __rcvr) -> __opstate<_Receiver>
        requires sender_in<__child_of<_Sender>, __make_env_t<__fwd_env_t<env_of_t<_Receiver>>>>
      {
        return __opstate<_Receiver>{static_cast<_Receiver&&>(__rcvr)};
      };

      template <class _Self>
        requires(!dependent_sender<__child_of<_Self>>)
      static consteval auto __get_completion_signatures()
      {
        static_assert(__sender_for<_Self, with_frame_allocator_t>);
        return STDEXEC::get_completion_signatures<__child_of<_Self>>();
      }

      template <class _Self, class _Env>
      static consteval auto __get_completion_signatures()
      {
        static_assert(__sender_for<_Self, with_frame_allocator_t>);
        return STDEXEC::get_completion_signatures<__child_of<_Self>, __make_env_t<_Env>>();
      }
    };
  }  // namespace __with_frame_alloc

  using __with_frame_alloc::with_frame_allocator_t;
  inline constexpr with_frame_allocator_t with_frame_allocator{};
}  // namespace experimental::execution

namespace exec = experimental::execution;

namespace STDEXEC
{
  template <>
  struct __sexpr_impl<exec::with_frame_allocator_t> : exec::__with_frame_alloc::__impls
  {};
}  // namespace STDEXEC
