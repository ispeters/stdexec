module;
#include <cassert>
#include <cstdarg>
#include <cstdio>
export module stdexec;
#define STDEXEC_IN_MODULE_PURVIEW
#include "__detail/__read_env.hpp"
#include "__detail/__stopped_as_error.hpp"
#include "__detail/__task_scheduler.hpp"
