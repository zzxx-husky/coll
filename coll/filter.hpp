#pragma once

#include "base.hpp"
#include "traits.hpp"

namespace coll {
struct FilterArgsTag {};

template<typename F>
struct FilterArgs {
  using TagType = FilterArgsTag;

  F filter;
};

template<typename F>
inline FilterArgs<F> filter(F lambda) { return {lambda}; }

template<typename Parent, typename Args>
struct Filter {
  using InputType = typename traits::remove_cvr_t<Parent>::OutputType;
  using OutputType = InputType;

  Parent parent;
  Args args;

  template<typename Child>
  struct Execution : public Child {
    template<typename ... X>
    Execution(const Args& args, X&& ... x):
      Child(std::forward<X>(x)...),
      args(args) {
    }

    Args args;

    inline void process(InputType e) {
      if (bool(args.filter(e))) {
        Child::process(std::forward<InputType>(e));
      }
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
  std::enable_if_t<std::is_same<typename A::TagType, FilterArgsTag>::value>* = nullptr,
  std::enable_if_t<traits::is_pipe_operator<P>::value>* = nullptr>
inline Filter<P, A>
operator | (Parent&& parent, Args&& args) {
  return {std::forward<Parent>(parent), std::forward<Args>(args)};
}
} // namespace coll
