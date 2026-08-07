export module foo;
export namespace bar {

template <class... Ts> struct box {};

template <class... Ts> struct holder {
#ifndef REPRO
  template <unsigned N> using at = Ts...[N];
#else
  template <unsigned N> using at = __type_pack_element<N, Ts...>;
#endif
};

template <class H> auto get(H) -> typename H::template at<0>;

template <class Fn, class H> void visit(H h) { Fn{}(get(h)); }

struct mfn { void operator()(box<>) {} };
}

namespace bar {
void seed() { visit<mfn, holder<box<>>>({}); }   // forms at<0> in the purview
}
