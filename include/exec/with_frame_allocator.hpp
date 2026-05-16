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

    struct __unsynchronized_recycling_resource : std::pmr::memory_resource
    {
      constexpr __unsynchronized_recycling_resource() noexcept = default;

      STDEXEC_IMMOVABLE(__unsynchronized_recycling_resource);

      constexpr ~__unsynchronized_recycling_resource()
      {
        for (auto* __chnk = __list_.front(); __chnk != nullptr;)
        {
          auto [__next, __size, __align] = *__chnk;
          ::operator delete(static_cast<void*>(__chnk), __size, __align);
          __chnk = __next;
        }
      }

      constexpr void* allocate(std::size_t __bytes, std::size_t __alignment)
      {
        STDEXEC_ASSERT(__bytes >= sizeof(__chunk));

        auto __align = static_cast<std::align_val_t>(__alignment);

        if (__list_.empty())
        {
          return ::operator new(__bytes, __align);
        }
        else
        {
          __chunk* __ret = __list_.pop_front();

          STDEXEC_ASSERT(__ret->__size_ == __bytes);
          STDEXEC_ASSERT(__ret->__align_ == __align);

          return __ret;
        }
      }

      constexpr void deallocate(void* __p, std::size_t __bytes, std::size_t __align) noexcept
      {
        __chunk* __chnk = new (__p) __chunk(__bytes, std::align_val_t(__align));

        __list_.push_front(__chnk);
      }

     private:
      struct __chunk
      {
        __chunk* __next_{nullptr};

        std::size_t      __size_;
        std::align_val_t __align_;

        constexpr explicit __chunk(std::size_t __size, std::align_val_t __align) noexcept
          : __size_(__size)
          , __align_(__align)
        {}
      };

      __intrusive_slist<&__chunk::__next_> __list_;

      constexpr void* do_allocate(std::size_t __bytes, std::size_t __align) final
      {
        return allocate(__bytes, __align);
      }

      constexpr void do_deallocate(void* __p, std::size_t __bytes, std::size_t __align) final
      {
        deallocate(__p, __bytes, __align);
      }

      constexpr bool do_is_equal(std::pmr::memory_resource const & __other) const noexcept final
      {
        return this == &__other;
      }
    };

    template <class _Resource>
    concept __memory_resource = requires(_Resource& __resource) {
      { __resource.allocate(std::size_t{}, std::size_t{}) } -> __same_as<void*>;
      { __resource.deallocate((void*) nullptr, std::size_t{}, std::size_t{}) } -> __same_as<void>;
    };

    template <__memory_resource _Resource = __unsynchronized_recycling_resource>
    struct __chunked_resource : std::pmr::memory_resource
    {
      constexpr __chunked_resource() noexcept = default;

      STDEXEC_IMMOVABLE(__chunked_resource);

      constexpr ~__chunked_resource() = default;

      constexpr void* allocate(std::size_t __bytes, std::size_t __align)
      {
        // delegate to the appropriate chunk if __bytes is small enough
        if (auto __chunk_index = __chunk_index_of(__bytes); __chunk_index < __chunk_count)
        {
          return __chunks_[__chunk_index].allocate(__bytes, __align);
        }
        else
        {
          // __bytes is bigger than the largest chunk so just punt to ::new
          return ::operator new(__bytes, static_cast<std::align_val_t>(__align));
        }
      }

      constexpr void deallocate(void* __p, std::size_t __bytes, std::size_t __align) noexcept
      {
        if (auto __chunk_index = __chunk_index_of(__bytes); __chunk_index < __chunk_count)
        {
          STDEXEC_TRY
          {
            // delegate deallocation to the appropriate chunk
            __chunks_[__chunk_index].deallocate(__p, __bytes, __align);
          }
          STDEXEC_CATCH(...)
          {
            // deallocate is required to "throw nothing" but is not marked noexcept...
            std::unreachable();
          }
        }
        else
        {
          // __p points at an allocation that's too big for us to have cached
          ::operator delete(__p, __bytes, static_cast<std::align_val_t>(__align));
        }
      }

     private:
      constexpr void* do_allocate(std::size_t __bytes, std::size_t __align) final
      {
        return allocate(__bytes, __align);
      }

      constexpr void
      do_deallocate(void* __p, std::size_t __bytes, std::size_t __align) noexcept final
      {
        return deallocate(__p, __bytes, __align);
      }

      constexpr bool do_is_equal(std::pmr::memory_resource const & __other) const noexcept final
      {
        return static_cast<std::pmr::memory_resource const *>(this) == &__other;
      }

      // the number of bytes in the smallest allocation
      static constexpr std::size_t __smallest_chunk = 64;
      // the number of allocation sizes before falling back to ::new
      static constexpr std::size_t __chunk_count = 10;

      static constexpr std::size_t __allocation_size_of(std::size_t __bytes) noexcept
      {
        // round __bytes up to the nearest power of two that is at least
        // __smallest_chunk
        return std::bit_ceil(__bytes | __smallest_chunk);
      }

      static constexpr std::size_t __chunk_index_of(std::size_t __bytes) noexcept
      {
        // compute the index into __chunks_ containing the cached allocations for
        // allocations of size __bytes; returns an index larger than the largest valid
        // index for values of __bytes whose allocations will not be cached
        return std::countr_zero(__allocation_size_of(__bytes)) - std::countr_zero(__smallest_chunk);
      }

      std::array<_Resource, __chunk_count> __chunks_{};
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

    template <class _Ty, __memory_resource _Resource = __chunked_resource<>>
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
      _Receiver                    __rcvr_;
      mutable __chunked_resource<> __resource_;
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
