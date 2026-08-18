module;

#include <cassert>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

export module stdexec.level_9;

import stdexec.level_0;
import stdexec.level_1;
import stdexec.level_2;
import stdexec.level_3;
import stdexec.level_4;
import stdexec.level_5;
import stdexec.level_6;
import stdexec.level_7;
import stdexec.level_8;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winclude-angled-in-module-purview"

#include <stdexec/__detail/__config.hpp>

#define STDEXEC_IN_MODULE_PURVIEW

#include <exec/system_context.hpp>

#pragma clang diagnostic pop
