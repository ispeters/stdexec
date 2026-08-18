module;

#include <cassert>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

export module stdexec.level_4;

import stdexec.level_0;
import stdexec.level_1;
import stdexec.level_2;
import stdexec.level_3;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winclude-angled-in-module-purview"

#include <stdexec/__detail/__config.hpp>

#define STDEXEC_IN_MODULE_PURVIEW

#include <exec/detail/numa.hpp>
#include <exec/sequence/iterate.hpp>
#include <stdexec/__detail/__any_allocator.hpp>
#include <stdexec/__detail/__counting_scopes.hpp>
#include <stdexec/__detail/__spawn.hpp>
#include <stdexec/__detail/__spawn_future.hpp>
#include <stdexec/__detail/__task.hpp>

#pragma clang diagnostic pop
