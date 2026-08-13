module;

#include <cassert>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

export module stdexec.level_2;

import stdexec.level_0;
import stdexec.level_1;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winclude-angled-in-module-purview"

#include <stdexec/__detail/__config.hpp>

#define STDEXEC_IN_MODULE_PURVIEW

#include <stdexec/__detail/__affine.hpp>
#include <stdexec/__detail/__any_allocator.hpp>
#include <stdexec/__detail/__as_awaitable.hpp>
#include <stdexec/__detail/__bulk.hpp>
#include <stdexec/__detail/__completion_signatures.hpp>
#include <stdexec/__detail/__continues_on.hpp>
#include <stdexec/__detail/__counting_scopes.hpp>
#include <stdexec/__detail/__debug.hpp>
#include <stdexec/__detail/__finally.hpp>
#include <stdexec/__detail/__inline_scheduler.hpp>
#include <stdexec/__detail/__into_variant.hpp>
#include <stdexec/__detail/__let.hpp>
#include <stdexec/__detail/__parallel_scheduler.hpp>
#include <stdexec/__detail/__parallel_scheduler_backend.hpp>
#include <stdexec/__detail/__queries.hpp>
#include <stdexec/__detail/__read_env.hpp>
#include <stdexec/__detail/__run_loop.hpp>
#include <stdexec/__detail/__schedule_from.hpp>
#include <stdexec/__detail/__schedulers.hpp>
#include <stdexec/__detail/__scope.hpp>
#include <stdexec/__detail/__sequence.hpp>
#include <stdexec/__detail/__spawn.hpp>
#include <stdexec/__detail/__spawn_common.hpp>
#include <stdexec/__detail/__spawn_future.hpp>
#include <stdexec/__detail/__starts_on.hpp>
#include <stdexec/__detail/__stop_when.hpp>
#include <stdexec/__detail/__stopped_as_optional.hpp>
#include <stdexec/__detail/__submit.hpp>
#include <stdexec/__detail/__sync_wait.hpp>
#include <stdexec/__detail/__task_scheduler.hpp>
#include <stdexec/__detail/__then.hpp>
#include <stdexec/__detail/__unstoppable.hpp>
#include <stdexec/__detail/__upon_error.hpp>
#include <stdexec/__detail/__upon_stopped.hpp>
#include <stdexec/__detail/__when_all.hpp>
#include <stdexec/__detail/__with_awaitable_senders.hpp>
#include <stdexec/__detail/__write_env.hpp>
#include <stdexec/stop_token.hpp>

#pragma clang diagnostic pop
