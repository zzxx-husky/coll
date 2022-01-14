#pragma once

#include "base.hpp"

namespace coll {
struct InspectArgsTag {};

template<typename P>
struct InspectArgs {
  using TagType = InspectArgsTag;

  P process;
};

template<typename P>
InspectArgs<P> inspect(P process) { return {std::forward<P>(process)}; }

template<typename Parent, typename Args>
struct Inspect {
  using InputType = typename traits::remove_cvr_t<Parent>::OutputType;
  using OutputType = InputType;

  Parent parent;
  Args args;

  template<typename Child>
  struct Execution : public Child {
    Args args;

    template<typename ... X>
    Execution(const Args& args, X&& ... x):
      args(args),
      Child(std::forward<X>(x)...) {
    }

    inline void process(InputType e) {
      args.process(e);
      Child::process(std::forward<InputType>(e));
    }
  };

  template<ExecutionType ET, typename Child, typename ... X>
  inline decltype(auto) wrap(X&& ... x) {
    return parent.template wrap<ET, Execution<Child>, Args&, X...>(
      args, std::forward<X>(x)...
    );
  }
};

template<typename Parent, typename Args,
  typename P = traits::remove_cvr_t<Parent>,
  typename A = traits::remove_cvr_t<Args>,
  std::enable_if_t<std::is_same<typename A::TagType, InspectArgsTag>::value>* = nullptr,
  std::enable_if_t<traits::is_pipe_operator<P>::value>* = nullptr>
inline Inspect<P, A>
operator | (Parent&& parent, Args&& args) {
  return {std::forward<Parent>(parent), std::forward<Args>(args)};
}

template<typename Optional, typename Args,
  typename O = traits::remove_cvr_t<Optional>,
  typename A = traits::remove_cvr_t<Args>,
  std::enable_if_t<std::is_same<typename A::TagType, InspectArgsTag>::value>* = nullptr,
  std::enable_if_t<traits::is_optional<O>::value>* = nullptr>
inline decltype(auto) operator | (Optional&& optional, Args&& args) {
  if (bool(optional)) {
    args.process(*optional);
  }
  return std::forward<Optional>(optional);
}
} // namespace coll

