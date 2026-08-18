module;

#include <cassert>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

export module stdexec.level_3;

import stdexec.level_0;
import stdexec.level_1;
import stdexec.level_2;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winclude-angled-in-module-purview"

#include <stdexec/__detail/__config.hpp>

#define STDEXEC_IN_MODULE_PURVIEW

#include <exec/detail/basic_sequence.hpp>
#include <exec/libdispatch_queue.hpp>
#include <exec/sequence_senders.hpp>
#include <exec/timed_scheduler.hpp>
#include <exec/trampoline_scheduler.hpp>
#include <stdexec/__detail/__any.hpp>
#include <stdexec/__detail/__associate.hpp>
#include <stdexec/__detail/__inline_scheduler.hpp>
#include <stdexec/__detail/__optional.hpp>
#include <stdexec/__detail/__receiver_adaptor.hpp>
#include <stdexec/__detail/__sender_ref.hpp>
#include <stdexec/__detail/__spawn_common.hpp>
#include <stdexec/__detail/__stop_when.hpp>
#include <stdexec/__detail/__storage.hpp>
#include <stdexec/__detail/__submit.hpp>
#include <stdexec/__detail/__transform_completion_signatures.hpp>
#include <stdexec/__detail/__with_awaitable_senders.hpp>

#pragma clang diagnostic pop
