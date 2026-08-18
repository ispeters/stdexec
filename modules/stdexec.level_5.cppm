module;

#include <cassert>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

export module stdexec.level_5;

import stdexec.level_0;
import stdexec.level_1;
import stdexec.level_2;
import stdexec.level_3;
import stdexec.level_4;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winclude-angled-in-module-purview"

#include <stdexec/__detail/__config.hpp>

#define STDEXEC_IN_MODULE_PURVIEW

#include <exec/at_coroutine_exit.hpp>
#include <exec/function.hpp>
#include <exec/sequence/any_sequence_of.hpp>
#include <exec/sequence/merge.hpp>
#include <exec/static_thread_pool.hpp>
#include <stdexec/__detail/__parallel_scheduler_backend.hpp>
#include <stdexec/__detail/__task_scheduler.hpp>

#pragma clang diagnostic pop
