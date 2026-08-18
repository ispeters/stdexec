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

#include <exec/asio/completion_token.hpp>
#include <exec/asio/use_sender.hpp>
#include <exec/create.hpp>
#include <exec/detail/basic_sequence.hpp>
#include <exec/env.hpp>
#include <exec/fork_join.hpp>
#include <exec/get_frame_allocator.hpp>
#include <exec/into_tuple.hpp>
#include <exec/just_from.hpp>
#include <exec/libdispatch_queue.hpp>
#include <exec/materialize.hpp>
#include <exec/reschedule.hpp>
#include <exec/sequence/empty_sequence.hpp>
#include <exec/sequence_senders.hpp>
#include <exec/single_thread_context.hpp>
#include <exec/start_now.hpp>
#include <exec/timed_scheduler.hpp>
#include <exec/timed_thread_scheduler.hpp>
#include <exec/trampoline_scheduler.hpp>
#include <exec/unless_stop_requested.hpp>
#include <exec/variant_sender.hpp>
#include <exec/when_any.hpp>
#include <stdexec/__detail/__any.hpp>
#include <stdexec/__detail/__associate.hpp>
#include <stdexec/__detail/__inline_scheduler.hpp>
#include <stdexec/__detail/__optional.hpp>
#include <stdexec/__detail/__receiver_adaptor.hpp>
#include <stdexec/__detail/__receiver_ref.hpp>
#include <stdexec/__detail/__sender_ref.hpp>
#include <stdexec/__detail/__spawn_common.hpp>
#include <stdexec/__detail/__stop_when.hpp>
#include <stdexec/__detail/__storage.hpp>
#include <stdexec/__detail/__transform_completion_signatures.hpp>
#include <stdexec/__detail/__with_awaitable_senders.hpp>

#pragma clang diagnostic pop
