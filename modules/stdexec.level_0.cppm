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

#include <stdexec/__detail/__awaitable.hpp>
#include <stdexec/__detail/__completion_behavior.hpp>
#include <stdexec/__detail/__config.hpp>
#include <stdexec/__detail/__diagnostics.hpp>
#include <stdexec/__detail/__domain.hpp>
#include <stdexec/__detail/__execution_fwd.hpp>
#include <stdexec/__detail/__sender_concepts.hpp>
#include <stdexec/__detail/__spin_loop_pause.hpp>
#include <stdexec/__detail/__tag_invoke.hpp>
#include <stdexec/__detail/__transform_sender.hpp>
#include <stdexec/__detail/__tuple.hpp>
#include <stdexec/__detail/__typeinfo.hpp>
#include <stdexec/coroutine.hpp>

#pragma clang diagnostic pop
