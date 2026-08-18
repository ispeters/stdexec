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

#include <exec/__memory_resource_adaptor.hpp>
#include <exec/detail/bwos_lifo_queue.hpp>
#include <exec/detail/intrusive_heap.hpp>
#include <stdexec/__detail/__atomic_intrusive_queue.hpp>
#include <stdexec/__detail/__intrusive_mpsc_queue.hpp>
#include <stdexec/__detail/__intrusive_queue.hpp>
#include <stdexec/__detail/__intrusive_slist.hpp>
#include <stdexec/__detail/__memory.hpp>
#include <stdexec/__detail/__meta.hpp>
#include <stdexec/__detail/__static_vector.hpp>
#include <stdexec/functional.hpp>

#pragma clang diagnostic pop
