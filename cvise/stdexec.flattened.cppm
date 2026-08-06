module;
#include <cassert>
#include <cstdarg>
#include <cstdio>
export module stdexec;
#define STDEXEC_IN_MODULE_PURVIEW

#include "__detail/__config.hpp"

namespace STDEXEC {
  // Minimal stand-in for __tuple: a bare class template with one static
  // member, nothing else. Established (via the ladder investigation) to
  // reproduce the bug just as well as the real __tuple<> -- __tuple is not
  // special. Kept over __tuple itself because it removes __tuple.hpp,
  // __box, __tupl_base, __make_indices, and the explicit specialization
  // entirely from what cvise has to reduce.
  export template <class... _Ts>
  struct my_tuple {
    static constexpr int size = sizeof...(_Ts);
  };
} // namespace STDEXEC

// Known to compile as-is; not hand-trimmed. Pulls in __variant.hpp (via
// __let.hpp) along with the meta/concepts machinery __variant.hpp itself
// relies on without including directly. Let cvise find the minimal subset
// rather than re-deriving it by hand and risking unrelated errors.
#include "__detail/__let.hpp"
#include "__detail/__read_env.hpp"
#include "__just.hpp"

namespace STDEXEC {
  // THE SEED. Mirrors the ladder's step 2 exactly: __no_init construction,
  // __emplace_from, __visit -- over my_tuple<> rather than __tuple<>. This
  // is the configuration confirmed to reproduce the bug. Nothing else in
  // this file is load-bearing; cvise should be able to remove everything
  // except this and whatever __variant.hpp needs to define __visit_alt.
  inline void __seed() {
    __variant<my_tuple<>> __v{__no_init};
    __v.__emplace_from([] { return my_tuple<>{}; });
    __visit([](auto &...) noexcept {}, __v);
  }
} // namespace STDEXEC
