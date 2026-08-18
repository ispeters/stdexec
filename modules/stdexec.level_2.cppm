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

#include <exec/detail/shared.hpp>
#include <exec/ensure_started.hpp>
#include <exec/execute.hpp>
#include <exec/split.hpp>
#include <exec/start_detached.hpp>
#include <stdexec/__detail/__affine.hpp>
#include <stdexec/__detail/__as_awaitable.hpp>
#include <stdexec/__detail/__basic_sender.hpp>
#include <stdexec/__detail/__bulk.hpp>
#include <stdexec/__detail/__completion_behavior.hpp>
#include <stdexec/__detail/__completion_signatures.hpp>
#include <stdexec/__detail/__connect.hpp>
#include <stdexec/__detail/__connect_awaitable.hpp>
#include <stdexec/__detail/__continues_on.hpp>
#include <stdexec/__detail/__debug.hpp>
#include <stdexec/__detail/__diagnostics.hpp>
#include <stdexec/__detail/__domain.hpp>
#include <stdexec/__detail/__env.hpp>
#include <stdexec/__detail/__execution_fwd.hpp>
#include <stdexec/__detail/__finally.hpp>
#include <stdexec/__detail/__get_completion_signatures.hpp>
#include <stdexec/__detail/__into_variant.hpp>
#include <stdexec/__detail/__just.hpp>
#include <stdexec/__detail/__let.hpp>
#include <stdexec/__detail/__on.hpp>
#include <stdexec/__detail/__operation_states.hpp>
#include <stdexec/__detail/__queries.hpp>
#include <stdexec/__detail/__query.hpp>
#include <stdexec/__detail/__read_env.hpp>
#include <stdexec/__detail/__receivers.hpp>
#include <stdexec/__detail/__run_loop.hpp>
#include <stdexec/__detail/__schedule_from.hpp>
#include <stdexec/__detail/__schedulers.hpp>
#include <stdexec/__detail/__scope.hpp>
#include <stdexec/__detail/__sender_adaptor_closure.hpp>
#include <stdexec/__detail/__sender_concepts.hpp>
#include <stdexec/__detail/__sender_introspection.hpp>
#include <stdexec/__detail/__sequence.hpp>
#include <stdexec/__detail/__starts_on.hpp>
#include <stdexec/__detail/__stop_token.hpp>
#include <stdexec/__detail/__stopped_as_error.hpp>
#include <stdexec/__detail/__stopped_as_optional.hpp>
#include <stdexec/__detail/__sync_wait.hpp>
#include <stdexec/__detail/__then.hpp>
#include <stdexec/__detail/__transfer_just.hpp>
#include <stdexec/__detail/__transform_sender.hpp>
#include <stdexec/__detail/__unstoppable.hpp>
#include <stdexec/__detail/__upon_error.hpp>
#include <stdexec/__detail/__upon_stopped.hpp>
#include <stdexec/__detail/__utility.hpp>
#include <stdexec/__detail/__variant.hpp>
#include <stdexec/__detail/__when_all.hpp>
#include <stdexec/__detail/__write_env.hpp>
#include <stdexec/stop_token.hpp>

#pragma clang diagnostic pop
