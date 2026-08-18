module;

#include <cassert>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

export module stdexec.level_6;

import stdexec.level_0;
import stdexec.level_1;
import stdexec.level_2;
import stdexec.level_3;
import stdexec.level_4;
import stdexec.level_5;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winclude-angled-in-module-purview"

#include <stdexec/__detail/__config.hpp>

#define STDEXEC_IN_MODULE_PURVIEW

#include <exec/on_coro_disposition.hpp>
#include <exec/task.hpp>
#include <exec/thread_pool_base.hpp>
#include <stdexec/__detail/__parallel_scheduler_default_impl.hpp>

#pragma clang diagnostic pop
