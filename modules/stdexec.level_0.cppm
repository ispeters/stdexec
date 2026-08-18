module;

#include <cassert>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

export module stdexec.level_0;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winclude-angled-in-module-purview"

#include <stdexec/__detail/__config.hpp>

#define STDEXEC_IN_MODULE_PURVIEW

#include <exec/completion_signatures.hpp>
#include <exec/detail/atomic_intrusive_queue.hpp>
#include <exec/detail/xorshift.hpp>
#include <exec/sequence.hpp>
#include <stdexec/__detail/__awaitable.hpp>
#include <stdexec/__detail/__config.hpp>
#include <stdexec/__detail/__spin_loop_pause.hpp>
#include <stdexec/__detail/__tag_invoke.hpp>
#include <stdexec/__detail/__tuple.hpp>
#include <stdexec/__detail/__typeinfo.hpp>
#include <stdexec/coroutine.hpp>

#pragma clang diagnostic pop
