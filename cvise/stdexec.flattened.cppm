module;
#include <cassert>
#include <cstdarg>
#include <cstdio>
export module stdexec;
#define STDEXEC_IN_MODULE_PURVIEW
#include "__bulk.hpp"
#include "__detail/__let.hpp"
#include "__detail/__read_env.hpp"
#include "__just.hpp"
#include "__parallel_scheduler_backend.hpp"
namespace STDEXEC {
namespace __task {
using namespace parallel_scheduler_replacement;
template <class> struct __bulk_sender;
template <class> struct __itask_scheduler_backend;
template <class _Base>
using __itask_scheduler_base_t = __any::__interface_base<
    __itask_scheduler_backend, _Base,
    __any::__extends<__iparallel_scheduler_backend, __any::__icopyable>>;
template <class _Base>
struct __itask_scheduler_backend : __itask_scheduler_base_t<_Base> {
  using __itask_scheduler_base_t<_Base>::__itask_scheduler_base_t;
};
struct __any_task_scheduler_backend : __any::__any<__itask_scheduler_backend> {
  using __any ::__any;
};
struct __unstoppable_env : __detail::__proxy_env {};
template <class _Sch> struct __just_with_scheduler {
  struct __env {
    template <class... _Env>
    auto query(get_completion_scheduler_t<set_value_t>,
               _Env... __env) const noexcept {
      return get_completion_scheduler<set_value_t>(__sch_, __env...);
    }
    _Sch __sch_;
  };
  using sender_concept = sender_tag;
  template <class> static consteval auto get_completion_signatures() {
    return completion_signatures<set_value_t()>{};
  }
  auto get_env() const noexcept -> __env;
  _Sch __sch_;
};
template <bool _Unstoppable>
using __env_t = __if_c<_Unstoppable, __unstoppable_env, __detail::__proxy_env>;
} // namespace __task
struct task_scheduler_domain {
  template <__sender_for _Sndr, class _Env>
  auto transform_sender(set_value_t, _Sndr __sndr, _Env __env) {
    auto __sch = get_completion_scheduler<set_value_t>(get_env(__sndr), __env);
    return __task::__bulk_sender{__sndr, __sch};
  }
};
struct task_scheduler {
  template <class, class> class __backend_for;
  template <__not_same_as<task_scheduler> _Sch>
  task_scheduler(_Sch __sch, int __alloc = {})
      : __backend_(__backend_for{__sch, __alloc}) {}
  auto schedule() const -> int;
  bool operator==(task_scheduler) const;
  auto query(get_completion_domain_t<set_value_t>) const {
    return task_scheduler_domain{};
  }
  __task::__any_task_scheduler_backend __backend_;
};
namespace __task {
template <int, class _Fn>
auto __get_execute_bulk_fn(bulk_unchunked_t, _Fn, size_t, size_t, size_t) {
  return [](auto) {};
}
template <class _BulkTag, class _Policy, class _Fn, class _Rcvr, class _Values>
struct __bulk_state
    : __detail::__receiver_proxy_base<_Rcvr, bulk_item_receiver_proxy> {
  __bulk_state(_Rcvr __rcvr, size_t, _Fn __fn, __any_task_scheduler_backend)
      : __bulk_state::__receiver_proxy_base(__rcvr), __fn_(__fn) {}
  void set_value() noexcept;
  void execute(size_t __begin, size_t __end) noexcept STDEXEC_TRY {
    using __policy_t = std::remove_cvref_t<decltype(__declval<_Policy>)>;
    constexpr bool __parallelize =
        __same_as<__policy_t, parallel_unsequenced_policy>;
    __visit(__get_execute_bulk_fn<__parallelize>(_BulkTag(), __fn_, __shape_,
                                                 __begin, __end),
            __values_);
  }
  STDEXEC_CATCH_ALL {}
  _Fn __fn_;
  size_t __shape_;
  _Values __values_{__no_init};
};
template <class _BulkTag, class _Policy, class _Sndr, class _Fn, class _Rcvr>
struct __bulk_opstate {
  __bulk_opstate(_Sndr, size_t __shape, _Fn __fn, _Rcvr __rcvr,
                 __any_task_scheduler_backend __backend)
      : __state_{__rcvr, __shape, __fn, __backend} {}
  using __values_t =
      value_types_of_t<_Sndr, _Rcvr, __decayed_tuple, __uniqued_variant>;
  __bulk_state<_BulkTag, _Policy, _Fn, _Rcvr, __values_t> __state_;
};
template <class _Sndr> struct __bulk_sender {
  template <class _Rcvr> auto connect(_Rcvr __rcvr) {
    auto [__tag, __data, __child] = __sndr_;
    auto [__pol, __shape, __fn] = __data;
    __bulk_opstate<decltype(__tag), decltype(__pol), decltype(__child),
                   decltype(__fn), _Rcvr>{__child, __shape, __fn, __rcvr,
                                          __attrs_.__sched_.__backend_};
  }
  _Sndr __sndr_;
  __sched_attrs<task_scheduler> __attrs_;
};
struct __bulk_unchunked_fn {
  void operator()(size_t);
  bulk_item_receiver_proxy &__rcvr_;
};
template <class, class _Sndr, class _Env> struct __opstate {
  using __rcvr_proxy_t = receiver_proxy;
  using __proxy_rcvr_t = __detail::__proxy_receiver<__rcvr_proxy_t, _Env>;
  using __child_opstate_t = connect_result_t<_Sndr, __proxy_rcvr_t>;
};
} // namespace __task
template <class _Sch, class _Alloc> struct task_scheduler::__backend_for {
  template <class _Env, class _RcvrProxy, class _Sndr>
  void __schedule(_RcvrProxy &, _Sndr, std::span<std::byte>) STDEXEC_TRY {
    using __opstate_t = __task::__opstate<_Alloc, _Sndr, _Env>;
    sizeof(__opstate_t);
  }
  STDEXEC_CATCH_ALL {}
  __backend_for(_Sch, _Alloc);
  void schedule(parallel_scheduler_replacement::receiver_proxy &,
                std::span<std::byte>);
  void schedule_bulk_chunked(
      size_t, parallel_scheduler_replacement::bulk_item_receiver_proxy &,
      std::span<std::byte>);
  void schedule_bulk_unchunked(
      size_t __count,
      parallel_scheduler_replacement::bulk_item_receiver_proxy &__rcvr_proxy,
      std::span<std::byte> __storage) {
    auto __sndr = bulk_unchunked(__task::__just_with_scheduler{__sch_}, __count,
                                 __task::__bulk_unchunked_fn{__rcvr_proxy});
    __schedule<__task::__env_t<true>>(__rcvr_proxy, __sndr, __storage);
  }
  _Sch __sch_;
};
namespace __detail {
template <class _Rcvr, class _Proxy, bool _Infallible>
constexpr void __receiver_proxy_base<_Rcvr, _Proxy, _Infallible>::__query(
    get_start_scheduler_t, __type_index, void *__dest) const noexcept {
  auto __val = *static_cast<std::optional<task_scheduler> *>(__dest);
  __val.emplace(inline_scheduler{});
}
} // namespace __detail
} // namespace STDEXEC