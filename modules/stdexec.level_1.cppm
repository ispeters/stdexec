module;

#include <cassert>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

export module stdexec.level_1;

import stdexec.level_0;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winclude-angled-in-module-purview"

#include <stdexec/__detail/__config.hpp>

#define STDEXEC_IN_MODULE_PURVIEW

#include <stdexec/__detail/__any.hpp>
#include <stdexec/__detail/__atomic_intrusive_queue.hpp>
#include <stdexec/__detail/__basic_sender.hpp>
#include <stdexec/__detail/__connect.hpp>
#include <stdexec/__detail/__connect_awaitable.hpp>
#include <stdexec/__detail/__env.hpp>
#include <stdexec/__detail/__intrusive_mpsc_queue.hpp>
#include <stdexec/__detail/__intrusive_queue.hpp>
#include <stdexec/__detail/__intrusive_slist.hpp>
#include <stdexec/__detail/__just.hpp>
#include <stdexec/__detail/__meta.hpp>
#include <stdexec/__detail/__operation_states.hpp>
#include <stdexec/__detail/__optional.hpp>
#include <stdexec/__detail/__query.hpp>
#include <stdexec/__detail/__receivers.hpp>
#include <stdexec/__detail/__sender_adaptor_closure.hpp>
#include <stdexec/__detail/__utility.hpp>
#include <stdexec/__detail/__variant.hpp>
#include <stdexec/functional.hpp>

#pragma clang diagnostic pop
