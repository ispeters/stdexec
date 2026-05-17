/*
 * Copyright (c) 2026 Ian Petersen
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

#include <exec/function.hpp>
#include <exec/with_frame_allocator.hpp>

#include <catch2/catch_all.hpp>

#include <stdexec/execution.hpp>

namespace ex = STDEXEC;

namespace
{
  TEST_CASE("with_frame_allocator builds", "[adaptors][with_frame_allocator]")
  {
    ex::sync_wait(exec::with_frame_allocator(exec::function<void() noexcept>{
      []() noexcept
      {
        return ex::read_env(exec::get_frame_allocator)
             | ex::then([](auto alloc) noexcept { REQUIRE(sizeof(alloc) > 0); });
      }}));
  }

  TEST_CASE("with_frame_allocator forwards environment queries", "[adaptors][with_frame_allocator]")
  {
    ex::sync_wait(exec::with_frame_allocator(ex::read_env(ex::get_scheduler)));
  }
}  // namespace
