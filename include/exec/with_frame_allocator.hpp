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

    template <auto _Next>
    struct __atomic_intrusive_slist;

    template <class _Item, _Item* _Item::* _Next>
    struct __atomic_intrusive_slist<_Next>
    {
      constexpr __atomic_intrusive_slist() noexcept = default;

      STDEXEC_IMMOVABLE(__atomic_intrusive_slist);

      constexpr ~__atomic_intrusive_slist() = default;

      [[nodiscard]]
      constexpr auto empty() const noexcept -> bool
      {
        return front() == nullptr;
      }

      [[nodiscard]]
      constexpr auto front() const noexcept -> _Item*
      {
        return __head_.load(__std::memory_order_acquire);
      }

      // not nodiscard
      constexpr auto clear() noexcept -> _Item*
      {
        return __head_.exchange(nullptr, __std::memory_order_acq_rel);
      }

      [[nodiscard]]
      constexpr auto pop_front() noexcept -> _Item*
      {
        STDEXEC_ASSERT(!empty());
        auto __result = __head_.load(__std::memory_order_relaxed);

        while (!__head_.compare_exchange_weak(__result,
                                              __result->*_Next,
                                              // load-acquire and store-release on success
                                              // to consume pushes and publish pops
                                              __std::memory_order_acq_rel,
                                              // load-relaxed on failure
                                              __std::memory_order_relaxed))
        {
          // nothing
        }

        return __result;
      }

      constexpr void push_front(_Item* __item) noexcept
      {
        STDEXEC_ASSERT(__item != nullptr);
        auto* __expected = __head_.load(__std::memory_order_relaxed);

        do
        {
          __item->*_Next = __expected;
        }
        while (!__head_.compare_exchange_weak(__expected,
                                              __item,
                                              // store-release on success
                                              __std::memory_order_release,
                                              // load-relaxed on failure
                                              __std::memory_order_relaxed));
      }

     private:
      __std::atomic<_Item*> __head_{nullptr};
    };

    template <bool _Synchronized>
    struct __linked_stack_cache
    {
      constexpr __linked_stack_cache() noexcept = default;

      STDEXEC_IMMOVABLE(__linked_stack_cache);

      constexpr ~__linked_stack_cache()
      {
        STDEXEC_ASSERT(__list_.empty() && "you must invoke deallocate_all before the destructor");
      }

      constexpr auto try_get() noexcept -> void*
      {
        if (__list_.empty())
        {
          return nullptr;
        }
        else
        {
          return __list_.pop_front();
        }
      }

      constexpr void put(void* __p) noexcept
      {
        __list_.push_front(new (__p) __chunk);
      }

      template <class _Deleter>
        requires __nothrow_invocable<_Deleter const &, void*>
      constexpr void deallocate_all(_Deleter const & __deleter) noexcept
      {
        auto* __current = __list_.clear();
        while (__current != nullptr)
        {
          __deleter((void*) std::exchange(__current, __current->__next_));
        }
      }

     private:
      struct __chunk
      {
        __chunk* __next_;
      };

      template <auto _Next>
      using __list_t =
        __if_c<_Synchronized, __atomic_intrusive_slist<_Next>, __intrusive_slist<_Next>>;

      __list_t<&__chunk::__next_> __list_;
    };

    template <class _Resource>
    concept __allocation_cache = requires(_Resource& __rsrc) {
      { __rsrc.try_get() } noexcept -> __same_as<void*>;
      // maybe put should be try_put if bounded caches become a thing
      { __rsrc.put((void*) nullptr) } noexcept;
      {
        __rsrc.deallocate_all([](void*) noexcept {})
      } noexcept;
    };

    template <__allocation_cache _Cache = __linked_stack_cache<true>>
    struct __chunked_caching_resource : std::pmr::memory_resource
    {
      constexpr __chunked_caching_resource() noexcept = default;

      STDEXEC_IMMOVABLE(__chunked_caching_resource);

      constexpr ~__chunked_caching_resource()
      {
        std::size_t __size = __smallest_allocation;
        // capture __size by reference because we're going to change it
        auto __deleter = [&__size](void* __p) noexcept
        {
          __deallocate(__p, __size, __uniform_alignment);
        };

        for (auto& __cache: __caches_)
        {
          __cache.deallocate_all(__deleter);
          __size <<= 1;
        }
      }

      //! hide memory_resource::allocate so that users that know our concrete type can
      //! skip virtual dispatch; our implementation of do_allocate delegates here to
      //! satisfy the requirements of subclasses of memory_resource
      constexpr auto allocate(std::size_t __bytes, std::size_t __align = __uniform_alignment)  //
        -> void*
      {
        auto __cache_index = __cache_index_of(__bytes);

        // check if the request is suitably sized and aligned that we might be able to
        // serve it from one of our caches
        if (__cache_index < __cache_count && __align <= __uniform_alignment)
        {
          // try_get is expected to return nullptr if the cache is empty
          if (void* __allocation = __caches_[__cache_index].try_get())
          {
            // cache hit, yay!
            return __allocation;
          }
          else
          {
            // boo, cache miss
            // override the alignment so that cache entries are uniformly aligned
            return __allocate(__bytes, __uniform_alignment);
          }
        }

        // the request is either too big or over-aligned so it can't have been cached
        return __allocate(__bytes, __align);
      }

      //! hide memory_resource::deallocate so that users that know our concrete type can
      //! skip virtual dispatch; our implementation of do_deallocate delegates here to
      //! satisfy the requirements of subclasses of memory_resource
      constexpr void
      deallocate(void* __p, std::size_t __bytes, std::size_t __align = __uniform_alignment) noexcept
      {
        auto __cache_index = __cache_index_of(__bytes);

        if (__cache_index < __cache_count && __align <= __uniform_alignment)
        {
          // this allocation is suitably sized and aligned to be cached
          __caches_[__cache_index].put(__p);
        }
        else
        {
          // we don't cache over-aligned allocations or allocations that are "too big"
          __deallocate(__p, __bytes, __align);
        }
      }

     private:
      //! convenience wrapper for ::operator new;
      //! this saves us from casting size_t to align_val_t in more than one place and
      //! leaves open the possibility of supporting an "upstream" memory_resource other
      //! than new_delete_resource
      static constexpr auto __allocate(std::size_t __bytes, std::size_t __align)  //
        -> void*
      {
        return ::operator new(__bytes, static_cast<std::align_val_t>(__align));
      }

      //! convenience wrapper for ::operator delete
      //! provides the same benefits as __allocate
      static constexpr void
      __deallocate(void* __p, std::size_t __bytes, std::size_t __align) noexcept
      {
        ::operator delete(__p, __bytes, static_cast<std::align_val_t>(__align));
      }

      //! implement the memory_resource virtual interface
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

      //! the alignment of all cached allocations
      //! must be alignof(std::max_align_t) because it's used as the default for the
      //! alignment argument to allocate and deallocate, which must have the same
      //! signatures as the corresponding member functions on memory_resource
      static constexpr std::size_t __uniform_alignment = alignof(std::max_align_t);

      //! the number of bytes in the smallest cached allocation; must be a power of 2
      static constexpr std::size_t __smallest_allocation = 64;
      //! the number of allocation sizes before falling back to ::operator new
      static constexpr std::size_t __cache_count = 10;

      //! given a request for an allocation of size __bytes, how many bytes should we
      //! actually allocate?
      static constexpr std::size_t __allocation_size_of(std::size_t __bytes) noexcept
      {
        // round __bytes up to the nearest power of two that is at least
        // __smallest_allocation
        return std::bit_ceil(__bytes | __smallest_allocation);
      }

      //! given a request for an allocation of size __bytes, which cache should be
      //! responsible for caching the allocation?
      static constexpr std::size_t __cache_index_of(std::size_t __bytes) noexcept
      {
        // compute the index into __caches_ containing the cached allocations for
        // allocations of size __bytes; returns an index larger than the largest valid
        // index for values of __bytes whose allocations will not be cached
        return std::countr_zero(__allocation_size_of(__bytes))
             - std::countr_zero(__smallest_allocation);
      }

      //! the chunked caches
      //! each entry in the array caches allocations twice as big as the one prior, with
      //! the first entry caching allocations of size __smallest_allocation
      std::array<_Cache, __cache_count> __caches_{};
    };

    template <class _Receiver>
    struct __opstate
    {
      _Receiver                            __rcvr_;
      mutable __chunked_caching_resource<> __resource_;
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
      __join_env_t<prop<get_frame_allocator_t, __chunked_caching_resource<>*>, _Env...>;

    struct __impls : __sexpr_defaults
    {
      static constexpr auto __get_attrs =
        []<class _Child>(__ignore, __ignore, _Child const & __child) noexcept
      {
        return __sync_attrs{__child};
      };

      static constexpr auto __get_env =
        []<class _Receiver>(__ignore, __opstate<_Receiver> const & __state) noexcept
        -> __make_env_t<__fwd_env_t<env_of_t<_Receiver>>>
      {
        return __env::__join(prop(get_frame_allocator, std::addressof(__state.__resource_)),
                             __fwd_env(STDEXEC::get_env(__state.__rcvr_)));
      };

      static constexpr auto __get_state =
        []<class _Sender, class _Receiver>(_Sender const &,
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
        return STDEXEC::get_completion_signatures<__child_of<_Self>,
                                                  __make_env_t<__fwd_env_t<_Env>>>();
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
