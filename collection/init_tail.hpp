#pragma once

#include "base.hpp"
#include "reference.hpp"
#include "utils.hpp"

namespace coll {
template<bool InitOrTail, bool CacheByRef>
struct InitTailArgs {
  constexpr static std::string_view name = "init_tail";

  inline InitTailArgs<InitOrTail, true> cache_by_ref() { return {}; }

  static constexpr bool is_init = InitOrTail;
  static constexpr bool is_tail = !InitOrTail;

  template<typename Input>
  using ElemType = std::conditional_t<CacheByRef,
    Reference<traits::remove_vr_t<Input>>,
    std::optional<traits::remove_cvr_t<Input>>
  >;
};

inline InitTailArgs<true, false> init() { return {}; }

inline InitTailArgs<false, false> tail() { return {}; }

// return all elements except the last one
template<typename Parent, typename Args>
struct InitTail {
  using InputType = typename traits::remove_cvr_t<Parent>::OutputType;
  using OutputType = InputType;

  Parent parent;
  Args args;

  template<typename Child>
  struct InitExecution : public Child {
    Args args;
    typename Args::template ElemType<InputType> prev_elem;

    template<typename ... X>
    InitExecution(const Args& args, X&& ... x):
      args(args),
      Child(std::forward<X>(x)...) {
    }

    inline void process(InputType e) {
      if (likely(bool(prev_elem))) {
        Child::process(*prev_elem);
      }
      prev_elem = e;
    }
  };

  template<typename Child>
  struct TailExecution : public Child {
    Args args;
    bool skipped_head = false;

    template<typename ... X>
    TailExecution(const Args& args, X&& ... x):
      args(args),
      Child(std::forward<X>(x)...) {
    }

    inline void process(InputType e) {
      if (likely(skipped_head)) {
        Child::process(std::forward<InputType>(e));
      } else {
        skipped_head = true;
      }
    }
  };

  template<typename Child, typename ... X>
  inline decltype(auto) wrap(X&& ... x) {
    using Ctrl = traits::operator_control_t<Child>;
    if constexpr (Args::is_init != Ctrl::is_reversed) {
      return parent.template wrap<InitExecution<Child>, Args&, X...>(
        args, std::forward<X>(x) ...
      );
    } else {
      return parent.template wrap<TailExecution<Child>, Args&, X...>(
        args, std::forward<X>(x) ...
      );
    }
  }
};

template<typename Parent, typename Args,
  typename P = traits::remove_cvr_t<Parent>,
  typename A = traits::remove_cvr_t<Args>,
  std::enable_if_t<A::name == "init_tail">* = nullptr,
  std::enable_if_t<traits::is_pipe_operator<P>::value>* = nullptr>
inline InitTail<P, A>
operator | (Parent&& parent, Args&& args) {
  return {std::forward<Parent>(parent), std::forward<Args>(args)};
}
} // namespace coll

