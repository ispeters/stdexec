
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

#include "../stdexec/__detail/__atomic.hpp"
#include "../stdexec/__detail/__config.hpp"

#include "../stdexec/__detail/__prologue.hpp"

namespace experimental::execution
{
  namespace __aslist
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

  }  // namespace __aslist

  using __aslist::__atomic_intrusive_slist;
}  // namespace experimental::execution

namespace exec = experimental::execution;

#include "../stdexec/__detail/__epilogue.hpp"
