module;
#include <cassert>
#include <cstdarg>
#include <cstdio>
export module stdexec;
#define STDEXEC_IN_MODULE_PURVIEW
#include "__prologue.hpp"
#include "__query.hpp"
namespace STDEXEC {
template <class, class, class...>
STDEXEC_ATTRIBUTE()
constexpr auto __get_completion_behavior() noexcept {}
namespace __cmplsigs {
template <class = __mlist<>, class = __mlist<>, class = __mlist<>>
struct __partitions;
template <class... _ValueTuples, class... _Errors, class... _Stopped>
struct __partitions<__mlist<_ValueTuples...>, __mlist<_Errors...>,
                    __mlist<_Stopped...>> {
  template <class _Tuple, class _Variant>
  using __value_types =
      _Variant::template __f<__mapply<_Tuple, _ValueTuples>...>;
};
template <class> struct __partitioned_fold_fn;
template <> struct __partitioned_fold_fn<set_value_t> {
  template <class... _ValueTuples, class _Errors, class _Stopped, class _Values>
  auto operator()(__partitions<__mlist<>, _Errors, _Stopped>,
                  __undefined<_Values> &)
      -> __undefined<__partitions<__mlist<_ValueTuples..., _Values>>> &;
};
template <class _Partitioned, class _Tag, class... _Args>
auto operator*(__undefined<_Partitioned> &, _Tag(_Args...))
    -> __call_result_t<__partitioned_fold_fn<_Tag>, _Partitioned,
                       __undefined<__mlist<_Args...>> &>;
template <class _Partitioned>
auto __unpack_partitioned_completions(__undefined<_Partitioned> &)
    -> _Partitioned;
template <class... _Sigs>
using __partition_completion_signatures_t =
    decltype(__cmplsigs::__unpack_partitioned_completions(
        (__declval<__undefined<__partitions<>> &>() * ... *
         static_cast<_Sigs *>(nullptr))));
template <class _Completions>
using __partitions_of_t = _Completions::__partitioned::__t;
}
template <class _Sigs, class _Tuple = __qq<__decayed_std_tuple>,
          class _Variant = __qq<__std_variant>>
using __value_types_t =
    __cmplsigs::__partitions_of_t<_Sigs>::template __value_types<_Tuple,
                                                                 _Variant>;
template <class... _Sigs> struct completion_signatures {
  struct __partitioned {
    using __t = __cmplsigs::__partition_completion_signatures_t<_Sigs...>;
  };
};
namespace __detail {
template <class> struct __gather_sigs_fn;
template <> struct __gather_sigs_fn<set_value_t> {
  template <class _Sigs, class _Tuple, class _Variant>
  using __f = __value_types_t<_Sigs, _Tuple, _Variant>;
};
}
template <class _WantedTag, class _Sigs, class _Tuple, class _Variant>
using __gather_completions_t =
    __detail::__gather_sigs_fn<_WantedTag>::template __f<_Sigs, _Tuple,
                                                         _Variant>;
namespace __detail {
template <class, class> size_t __count_of;
}
}
#include "../functional.hpp"
#include "__env.hpp"
#include "__epilogue.hpp"
namespace STDEXEC {
namespace __detail {
template <__disposition> struct __completion_tag {};
}
struct set_value_t : __detail::__completion_tag<__disposition::__value> {};
namespace __detail {
template <class _Receiver>
concept __enable_receiver =
    (STDEXEC_PP_WHEN() __std::derived_from<typename _Receiver::receiver_concept,
                                           receiver_tag>);
}
template <class _Receiver>
concept receiver = __detail::__enable_receiver<__decay_t<_Receiver>> &&
                   __environment_provider<__cref_t<_Receiver>> &&
                   __nothrow_move_constructible<> &&
                   __std::constructible_from<__decay_t<_Receiver>>;
#define STDEXEC_GET_COMPLSIGS(...)                                             \
  __VA_ARGS__::template get_completion_signatures<__VA_ARGS__>()
namespace __cmplsigs {
template <class _Sender, class> auto __get_completion_signatures_helper() {
  return STDEXEC_GET_COMPLSIGS(_Sender);
}
}
template <class _Sender, class _Env> auto get_completion_signatures() {
  using __new_sndr_t = transform_sender_result_t<_Sender, _Env>;
  return __cmplsigs::__get_completion_signatures_helper<__new_sndr_t, _Env>();
}
template <class _Sender, class... _Env>
using __completion_signatures_of_t =
    decltype(get_completion_signatures<_Sender, _Env...>());
template <class _WantedTag, class _Sender, class _Env, class _Tuple,
          class _Variant>
using __gather_completions_of_t = __gather_completions_t<
    _WantedTag, __completion_signatures_of_t<_Sender, _Env>, _Tuple, _Variant>;
template <class _Sender, class _Env, template <class> class _Tuple,
          template <class> class _Variant>
using value_types_of_t =
    __value_types_t<__completion_signatures_of_t<_Sender, _Env>, __q<_Tuple>,
                    __q<_Variant>>;
template <class _Tag, class _Sender>
using __count_of = __msize_t<__detail::__count_of<_Tag, _Sender>>;
template <class _Sender>
concept sender = __std::move_constructible<_Sender>;
template <class _Tag, class _Sender>
concept __never_sends = __count_of<_Tag, _Sender>::value;
template <class _Tag, class _Sender>
using __never_sends_t = __mbool<__never_sends<_Tag, _Sender>>;
template <auto> struct __sexpr;
template <class _Tag, class _Data, class... _Child> struct __desc {
  using __tag = _Tag;
  using __data = _Data;
  using __children = __mlist<_Child...>;
  template <class _Fn> using __f = __minvoke<_Fn, _Tag, _Data, _Child...>;
};
namespace __detail {
template <class> extern __undefined<> __desc_of_v;
template <auto _Descriptor>
decltype(_Descriptor) __desc_of_v<__sexpr<_Descriptor>>;
}
template <class _Sender>
using __desc_of_t = decltype(__detail::__desc_of_v<__decay_t<_Sender>>);
template <class _Sender>
using __data_of =
    __copy_cvref_t<_Sender, typename __desc_of_t<_Sender>::__data>;
template <class _Sender, class _Continuation>
using __children_of =
    __mapply<__mtransform<__copy_cvref_fn<_Sender>, _Continuation>,
             typename __desc_of_t<_Sender>::__children>;
template <class _Sender>
using __child_of = __children_of<_Sender, __qq<__mfront>>;
template <class _Sender, class... _Tag>
concept __sender_for = (__std::same_as<_Sender, _Tag> && ...);
template <class _DomainOrTag, class _OpTag, class _Sender, class... _Env>
concept __has_transform_sender =
    requires(_DomainOrTag __tag, _Sender __sender, _Env... __env) {
      __tag.transform_sender(_OpTag(), __sender, __env...);
    };
template <class _DomainOrTag, class _OpTag, class _Sender, class... _Env>
using __transform_sender_result_t = decltype(_DomainOrTag{}.transform_sender(
    _OpTag(), __declval<_Sender>(), __declval<_Env>...));
struct default_domain {
  template <class _OpTag, class _Sender, class _Env>
  auto transform_sender(_OpTag, _Sender, _Env) -> _Sender;
};
namespace __detail {
template <class _Env>
using __starting_domain_t = __call_result_t<get_domain_t, _Env>;
template <class, class _Sender, class... _Env>
using __completing_domain_t =
    __call_result_t<get_completion_domain_t<>, _Sender, _Env...>;
template <class _Sch>
using __scheduler_domain_t = __call_result_t<get_completion_domain_t<>, _Sch>;
}
template <class> struct get_completion_domain_t {
  struct __read_query_t {
    template <class _Attrs, class... _Env>
      requires(__queryable_with<get_completion_domain_t, _Env> || ...) ||
              __queryable_with<_Attrs, get_completion_domain_t>
    auto operator()(_Attrs) {
      return __decay_t<__query_result_t<_Attrs, get_completion_domain_t>>{};
    }
  };
  template <class _Attrs, class... _Env> static auto __get_domain() {
    if constexpr (__callable<__read_query_t, _Attrs>) {
      using __domain_t = __call_result_t<__read_query_t, _Attrs>;
      return __domain_t{};
    } else
      return __call_result_t<get_domain_t, _Env...>{};
  }
  template <class _Attrs, class... _Env>
  using __result_t =
      __unless_one_of_t<decltype(__get_domain<_Attrs, _Env...>())>;
  template <class _Attrs, class... _Env>
  auto operator()(_Attrs, _Env...) -> __result_t<_Attrs, _Env...>;
};
struct get_domain_t {
  template <class _Env> auto operator()(_Env) {
    if constexpr (__callable<get_start_scheduler_t, _Env>) {
      using __sch_t = __call_result_t<get_start_scheduler_t, _Env>;
      using __cmpl_sch_t =
          __call_result_t<get_completion_scheduler_t<set_value_t>, __sch_t>;
      return __detail::__scheduler_domain_t<__cmpl_sch_t>{};
    }
  }
};
struct get_start_scheduler_t : __query<get_start_scheduler_t> {};
template <__completion_tag> struct get_completion_scheduler_t {
  template <class _Attrs> static consteval auto __get_declfn() {
    return __declfn<_Attrs>();
  }
  template <class _Attrs, auto _DeclFn = __get_declfn<_Attrs>()>
  auto operator()(_Attrs) -> __unless_one_of_t<decltype(_DeclFn())>;
};
template <class _Scheduler> struct __sched_attrs {
  _Scheduler __sched_;
};
namespace __detail {
template <class...> struct __mk_secondary_env_impl {
  template <class _CvFn, sender _Sender, class _Env>
  void operator()(_CvFn, _Sender, _Env);
};
}
template <class...> struct __mk_secondary_env_t {
  template <class, class>
  using __impl_t =
      __minvoke<__mremove_if<__mbind_back_q<__never_sends_t>,
                             __qq<__detail::__mk_secondary_env_impl>>>;
  template <class _CvFn, class _Sender, class _Env>
  auto operator()(_CvFn, _Sender, _Env)
      -> __call_result_t<__impl_t<_Sender, _Env>, _CvFn, _Sender, _Env>;
};
template <class _CvSender, class _Env>
using __secondary_env_t =
    __call_result_t<__mk_secondary_env_t<>, _CvSender, _CvSender, _Env>;
STDEXEC_P2300_NAMESPACE_END()
#define STDEXEC_SEXPR_DESCRIPTOR(_Tag, _Data, _Child)                          \
  __descriptor_fn_v<__desc<_Tag, _Data, _Child>>
namespace STDEXEC {
namespace __detail {
template <class _Domain, class _OpTag> struct __transform_sender_t {
  template <class _Sndr, class _Env>
  using __domain_for_t =
      __if_c<__has_transform_sender<_Domain, _OpTag, _Sndr, _Env>, _Domain,
             default_domain>;
  template <class _Sndr, class _Env> static consteval auto __get_declfn() {
    using __domain_t = __domain_for_t<_Sndr, _Env>;
    using __result_t =
        __transform_sender_result_t<__domain_t, _OpTag, _Sndr, _Env>;
    return __declfn<__result_t>();
  }
  template <class _Sndr, class _Env, auto _DeclFn = __get_declfn<_Sndr, _Env>()>
  auto operator()(_Sndr, _Env) -> decltype(_DeclFn());
};
}
struct transform_sender_t {
  template <class _Fn1, class _Fn2> struct __compose {
    template <class _Sndr, class _Env>
    auto operator()(_Sndr __sndr, _Env __env) const
        -> decltype(_Fn1()(_Fn2()(__sndr, __env), __env));
  };
  template <class _Sndr, class _Env>
  using __impl_fn_t = __compose<
      __detail::__transform_sender_t<__detail::__starting_domain_t<_Env>,
                                     start_t>,
      __detail::__transform_sender_t<
          __detail::__completing_domain_t<void, _Sndr, _Env>, set_value_t>>;
  template <class _Sndr, class _Env, auto _ImplFn = __impl_fn_t<_Sndr, _Env>{}>
  auto operator()(_Sndr __sndr, _Env __env) -> decltype(_ImplFn(__sndr, __env));
};
namespace __connect {
template <class _Sender, class _Receiver>
concept __with_static_member =
    requires(_Sender __sndr, __declfn_t<_Receiver> __rcvr) {
      _Sender::__static_connect(__sndr, __rcvr());
    };
template <class _Sender, class _Receiver>
concept __with_member = requires(__declfn_t<_Sender> __sndr, _Receiver __rcvr) {
  __sndr().connect(__rcvr);
};
#define STDEXEC_CONNECT_DECLFN_FOR(_EXPR) __declfn<decltype(_EXPR)>()
template <int> struct __connect_declfn {
  template <class _Sender, class _Receiver> static auto __get() {
    if constexpr (__with_static_member<_Sender, _Receiver>)
      return STDEXEC_CONNECT_DECLFN_FOR(
          _Sender ::__static_connect(__declval<_Sender>, _Receiver()));
    if constexpr (__with_member<_Sender, int>)
      return __declval<_Receiver>;
  }
};
template <class _Sender, class _Receiver>
using __connect_declfn_t =
    decltype(__connect_declfn<__nothrow_callable<_Receiver>>::template __get<
             transform_sender_result_t<_Sender, env_of_t<_Receiver>>,
             _Receiver>());
}
struct connect_t {
  template <class _Sender, class _Receiver,
            class _DeclFn = __connect::__connect_declfn_t<_Sender, _Receiver>>
  auto operator()(_Sender, _Receiver) const -> __call_result_t<_DeclFn>;
};
struct start_t {};
template <class _Descriptor> auto __descriptor_fn_v = _Descriptor{};
namespace __detail {
struct __defaults {
  static constexpr auto __connect = [] {};
};
}
template <class> struct __sexpr_impl : __detail::__defaults {};
template <class _Tag, class _Data, class... _Child>
using __sexpr_t = __sexpr<STDEXEC_SEXPR_DESCRIPTOR(_Tag, _Data, _Child...)>;
template <auto _DescriptorFn>
struct __sexpr : __minvoke<decltype(_DescriptorFn), __qq<__tuple>> {
  using __tag_t = decltype(_DescriptorFn)::__tag;
  template <class _Self> static auto get_completion_signatures() {
    using __self_t = __copy_cvref_t<_Self, __sexpr>;
    return __sexpr_impl<__tag_t>::template __get_completion_signatures<
        __self_t>();
  }
  template <class _Self, receiver _Receiver>
  static auto __static_connect(_Self &&, _Receiver)
      -> __result_of<__sexpr_impl<__tag_t>::__connect,
                     __copy_cvref_t<_Self, __sexpr>, _Receiver>;
};
namespace __detail {
template <class _Tag> struct __make_sexpr_t {
  template <class _Data, class... _Child> auto operator()(_Data, _Child...) {
    return __sexpr_t<_Tag, _Data, _Child...>{};
  }
};
}
template <class _Tag> __detail::__make_sexpr_t<_Tag> __make_sexpr;
struct {
  template <class _Fn, class _Variant, class... _As>
  void operator()(_Fn, _Variant __var,
                  _As...) noexcept(noexcept(__var.__visit(_Fn(), _Variant(),
                                                          __declval<_As>...)));
} __visit;
namespace __var {
template <size_t _Ny, class _Variant>
using __variant_alternative_t = __copy_cvref_t<
    _Variant, typename std::remove_reference_t<_Variant>::template __at_t<_Ny>>;
template <size_t _Ny, class _Variant>
auto __get(_Variant) -> __variant_alternative_t<_Ny, _Variant>;
template <size_t _Ny, class, class _Fn, class _Self, class... _Us>
auto __visit_alt(_Us... __us) {
  _Self __self;
  _Fn __fn;
  __fn(__us..., __var::__get<_Ny>(__self));
}
template <auto, class...> class __variant;
template <size_t... _Is, __indices<_Is...> _Idx, class... _Ts>
struct __variant<_Idx, _Ts...> {
  template <size_t _Ny> using __at_t = __m_at_c<_Ny, _Ts...>;
  template <class _Fn, class _Self, class... _Us>
    requires(__callable<_Fn, _Us..., __copy_cvref_t<_Self, _Ts>> && ...)
  auto __visit(_Fn, _Self, _Us...) {
    using __result_t =
        __call_result_t<_Fn, _Us..., __copy_cvref_t<_Self, __at_t<0>>>;
    __visit_alt<0, __result_t, _Fn, _Self, _Us...>;
  }
};
template <class... _Ts>
using __variant_base_t = __variant<__indices_for<>{}, _Ts...>;
}
STDEXEC_MODULE_EXPORT_AUTHORING template <class... _Ts>
struct __variant : __var::__variant_base_t<_Ts...> {};
template <class... Ts>
using __uniqued_variant = __mcall<__munique<__qq<__variant>>, Ts...>;
namespace __let {
template <class _SetTag, class _Sender, class>
using __env2_t = __secondary_env_t<_Sender, _SetTag>;
template <class, class, class, class, class... _Tuples> struct __opstate_base {
  using __env2_t = int;
  virtual void __start_next();
  __variant<_Tuples...> __args_;
};
STDEXEC_MODULE_EXPORT_AUTHORING auto __mk_result_sndr =
    []<class _Fun>(_Fun __fn...) { return __invoke(__fn); };
STDEXEC_MODULE_EXPORT_AUTHORING auto __start_next_fn =
    []<class _Fun, class _Receiver, class _Env2, class _Storage, class _Tuple>(
        _Fun __fn, _Receiver, _Env2, _Storage, _Tuple __tupl) {
      __apply(__mk_result_sndr, __tupl, __fn);
    };
STDEXEC_MODULE_EXPORT_AUTHORING template <class _SetTag, class _CvChild,
                                          class _Fun, class _Receiver,
                                          class... _Tuples>
struct __opstate
    : __opstate_base<_SetTag, _Fun, _Receiver,
                     __env2_t<_SetTag, _CvChild, _Receiver>, _Tuples...> {
  using __op_state_variant_t = __variant<>;
  void start();
  constexpr void __start_next() {
    __visit(__start_next_fn, this->__args_, this, this, this, __storage_);
  }
  __op_state_variant_t __storage_;
};
template <class _LetTag> struct __let_t {
  template <sender _Sender, __movable_value _Fun>
  auto operator()(_Sender __sndr, _Fun __fn) const {
    return __make_sexpr<_LetTag>(__fn, __sndr);
  }
};
template <class _LetTag> struct __impls {
  using __set_t = __t<_LetTag>;
  template <class _CvSender> using __fn_t = __decay_t<__data_of<_CvSender>>;
  template <class _CvSender, class _Receiver>
  using __opstate_t = __gather_completions_of_t<
      __set_t, __child_of<_CvSender>, _Receiver, __q<__decayed_tuple>,
      __mbind_front_q<__opstate, __set_t, __child_of<_CvSender>,
                      __fn_t<_CvSender>, _Receiver>>;
  static constexpr auto __connect =
      []<class _CvSender, class _Receiver>(
          _CvSender &&, _Receiver) -> __opstate_t<_CvSender, _Receiver> {};
};
}
struct let_value_t : __let::__let_t<let_value_t> {
  typedef STDEXEC::set_value_t __t;
};
template <> struct __sexpr_impl<let_value_t> : __let::__impls<let_value_t> {};
int par;
namespace __bulk {
template <class _Pol, class _Shape, class _Fun> struct __data {
  _Pol __pol_;
  _Shape __shape_;
  _Fun __fun_;
};
template <class> struct __generic_bulk_t {
  template <sender _Sender, typename _Policy, __std::integral _Shape,
            __std::copy_constructible _Fun>
  auto operator()(_Sender __sndr, _Policy __pol, _Shape __shape,
                  _Fun __fun) const {
    return __make_sexpr<int>(__data{__pol, __shape, __fun}, __sndr);
  }
  template <sender _Sender, __std::integral _Shape,
            __std::copy_constructible _Fun>
  auto operator()(_Sender __sndr, _Shape __shape, _Fun __fun) const {
    return (*this)(__sndr, par, __shape, __fun);
  }
};
}
struct bulk_unchunked_t : __bulk::__generic_bulk_t<bulk_unchunked_t> {};
template <class _JustTag> struct __impl : __detail::__defaults {
  using __set_tag_t = _JustTag::__tag_t;
  template <class _Sender> static auto __get_completion_signatures() {
    return completion_signatures<
        __mapply<__qf<__set_tag_t>, __decay_t<__data_of<_Sender>>>>{};
  }
};
struct just_t {
  typedef STDEXEC::set_value_t __tag_t;
  template <__movable_value... _Ts> auto operator()(_Ts... __ts) const {
    return __make_sexpr<just_t>(__tuple{__ts...});
  }
};
template <> struct __sexpr_impl<just_t> : __impl<just_t> {};
}
namespace STDEXEC::__any {
constexpr size_t __default_buffer_size = sizeof(void *);
template <template <class> class...> struct __extends {
  template <class _Base> using __f = _Base;
};
struct __iroot {
  template <class _Self> auto __value_(_Self) -> _Self;
};
template <template <class> class _Interface>
using __bases_of = _Interface<__iroot>::__bases_type;
template <template <class> class, class, class = __extends<>,
          size_t = __default_buffer_size, size_t = alignof(void *)>
struct __interface_base;
template <class> void __interface_cast();
struct {
  template <class _Ty> auto operator()(_Ty __arg) {
    return __arg.__value_(__arg);
  }
} __value;
template <template <class> class _Interface,
          class _BaseInterfaces = __bases_of<_Interface>>
using __iabstract = _Interface<__mcall1<_BaseInterfaces, __iroot>>;
template <class _Value> struct __box {
  template <class _Self> auto __value_(_Self) { return __val_; }
  _Value __val_;
};
template <class, template <class> class _Interface>
concept __already_implements = requires { __interface_cast<_Interface>; };
template <template <class> class _BaseInterface,
          template <class> class... _BaseInterfaces>
struct __extends<_BaseInterface, _BaseInterfaces...> {
  template <class _Base,
            class _BasesOfBase = __mcall1<__bases_of<_BaseInterface>, _Base>>
  using __f =
      __mcall1<__extends<>, __if_c<__already_implements<_Base, _BaseInterface>,
                                   _BasesOfBase, _BaseInterface<_BasesOfBase>>>;
};
template <class _Model, class _Allocator> _Model &__emplace_into(_Allocator);
template <template <class> class, class _Value, class>
struct __value_root_with_allocator : __box<_Value> {};
template <template <class> class _Interface, class _Value, class _Allocator>
struct __value_model
    : _Interface<__mcall1<
          __bases_of<_Interface>,
          __value_root_with_allocator<_Interface, _Value, _Allocator>>> {};
template <template <class> class _Interface>
struct __value_proxy_root : __iabstract<_Interface> {
  template <class _Value, class _Allocator>
  void __emplace_(std::allocator_arg_t, _Allocator) {
    using __model_type = __value_model<_Interface, _Value, _Allocator>;
    auto __model = __emplace_into<__model_type>(std::allocator_arg);
  }
};
template <template <class> class _Interface>
struct __value_proxy_model
    : _Interface<
          __mcall1<__bases_of<_Interface>, __value_proxy_root<_Interface>>> {};
template <template <class> class, class _Base, class _BaseInterfaces, size_t,
          size_t>
struct __interface_base : _Base {
  using __bases_type = _BaseInterfaces;
};
template <class _Value, template <class> class>
concept __model_of = !std::derived_from<_Value, __iroot>;
template <template <class> class _Interface>
struct __any : __value_proxy_model<_Interface> {
  template <__model_of<_Interface> _Value> __any(_Value __val) {
    (*this).template __emplace_<_Value>(std::allocator_arg, __val);
  }
};
}
namespace STDEXEC {
class task_scheduler;
template <class _Base>
struct __iparallel_scheduler_backend
    : __any::__interface_base<__iparallel_scheduler_backend, _Base> {
  virtual void schedule_bulk_unchunked(size_t __count, int __rcvr,
                                       std::span<std::byte> __scratch) {
    __any::__value(*this).schedule_bulk_unchunked(__count, __rcvr, __scratch);
  }
};
namespace __detail {
template <class, class, bool> struct __receiver_proxy_base {
  constexpr void __query(get_start_scheduler_t, __type_index,
                         void *) const noexcept;
};
struct __proxy_env {
  template <class _TaskScheduler = task_scheduler>
  auto query(get_start_scheduler_t) const -> _TaskScheduler;
};
template <class _Env> struct __proxy_receiver {
  auto get_env() const -> _Env;
};
}
namespace __task {
template <class> struct __bulk_sender;
template <class> struct __itask_scheduler_backend;
template <class _Base>
using __itask_scheduler_base_t =
    __any::__interface_base<__itask_scheduler_backend, _Base,
                            __any::__extends<__iparallel_scheduler_backend>>;
template <class _Base>
struct __itask_scheduler_backend : __itask_scheduler_base_t<_Base> {};
struct __any_task_scheduler_backend : __any::__any<__itask_scheduler_backend> {
  using __any::__any;
};
struct __unstoppable_env : __detail::__proxy_env {};
template <class _Sch> struct __just_with_scheduler {
  template <class> static auto get_completion_signatures() {
    return completion_signatures<set_value_t()>{};
  }
  _Sch __sch_;
};
template <bool _Unstoppable>
using __env_t = __if_c<_Unstoppable, __unstoppable_env, __detail::__proxy_env>;
}
struct task_scheduler_domain {
  template <__sender_for _Sndr, class _Env>
  auto transform_sender(set_value_t, _Sndr __sndr, _Env __env) {
    return __task::__bulk_sender{__sndr, __env};
  }
} task_scheduler___alloc;
struct task_scheduler {
  template <class, class> class __backend_for;
  template <__not_same_as<task_scheduler> _Sch>
  task_scheduler(_Sch __sch)
      : __backend_(__backend_for{__sch, task_scheduler___alloc}) {}
  auto query(get_completion_domain_t<>) const {
    return task_scheduler_domain{};
  }
  __task::__any_task_scheduler_backend __backend_;
};
namespace __task {
template <class, class, class, class, class _Values> struct __bulk_state {
  __bulk_state(int, size_t, int, __any_task_scheduler_backend) {
    __visit([](auto, auto) {}, __values_, this);
  }
  _Values __values_;
};
template <class _BulkTag, class _Policy, class _Sndr, class _Fn, class _Rcvr>
struct __bulk_opstate {
  __bulk_opstate(_Sndr, size_t __shape, int __fn, int __rcvr,
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
template <class, class _Sndr, class _Env> struct __opstate {
  using __proxy_rcvr_t = __detail::__proxy_receiver<_Env>;
  using __child_opstate_t = connect_result_t<_Sndr, __proxy_rcvr_t>;
};
}
template <class _Sch, class _Alloc> struct task_scheduler::__backend_for {
  template <class _Env, class _RcvrProxy, class _Sndr>
  void __schedule(_RcvrProxy, _Sndr, std::span<std::byte>) STDEXEC_TRY {
    using __opstate_t = __task::__opstate<_Alloc, _Sndr, _Env>;
    sizeof(__opstate_t);
  }
  STDEXEC_CATCH_ALL {}
  __backend_for(_Sch, _Alloc);
  void schedule_bulk_unchunked(size_t __count, int __rcvr_proxy,
                               std::span<std::byte> __storage) {
    auto __sndr =
        bulk_unchunked(__task::__just_with_scheduler{__sch_}, __count, int{});
    __schedule<__task::__env_t<true>>(__rcvr_proxy, __sndr, __storage);
  }
  int __sch_;
};
namespace __detail {
template <class _Rcvr, class _Proxy, bool _Infallible>
constexpr void __receiver_proxy_base<_Rcvr, _Proxy, _Infallible>::__query(
    get_start_scheduler_t, __type_index, void *__dest) const noexcept {
  auto __val = *static_cast<std::optional<task_scheduler> *>(__dest);
  __val.emplace(int{});
}
}
}
