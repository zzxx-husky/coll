#pragma once

#include "base.hpp"

namespace coll {
template<typename F>
struct FilterArgs {
  constexpr static std::string_view name = "filter";
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
      args(args),
      Child(std::forward<X>(x)...) {
    }

    Args args;

    inline void process(InputType e) {
      if (args.filter(e)) {
        Child::process(std::forward<InputType>(e));
      }
    }
  };

  template<typename Child, typename ... X>
  inline decltype(auto) wrap(X&& ... x) {
    return parent.template wrap<Execution<Child>, Args&, X...>(
      args, std::forward<X>(x)...
    );
  }
};

template<typename Parent, typename Args,
  std::enable_if_t<Args::name == "filter">* = nullptr,
  std::enable_if_t<traits::is_pipe_operator<Parent>::value>* = nullptr>
inline Filter<Parent, Args>
operator | (Parent&& parent, Args&& args) {
  return {std::forward<Parent>(parent), std::forward<Args>(args)};
}
} // namespace coll
