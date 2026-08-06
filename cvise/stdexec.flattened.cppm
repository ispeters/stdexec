module;
#include <cassert>
#include <cstdarg>
#include <cstdio>
export module stdexec;
#define STDEXEC_IN_MODULE_PURVIEW
#include "__detail/__config.hpp"
namespace STDEXEC {
export template <class...> struct my_tuple {};
} // namespace STDEXEC
#include "__detail/__let.hpp"
#include "__detail/__read_env.hpp"
namespace STDEXEC {
void __seed() {
  __variant<my_tuple<>> __v{__no_init};
  __visit([](auto) {}, __v);
}
} // namespace STDEXEC
