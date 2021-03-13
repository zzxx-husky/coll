#pragma once

#include "base.hpp"

namespace coll {
template<typename P>
struct InspectArgs {
  constexpr static std::string_view name = "inspect";
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

  template<typename Child, typename ... X>
  inline decltype(auto) wrap(X&& ... x) {
    return parent.template wrap<Execution<Child>, Args&, X...>(
      args, std::forward<X>(x)...
    );
  }
};

template<typename Parent, typename Args,
  std::enable_if_t<Args::name == "inspect">* = nullptr,
  std::enable_if_t<traits::is_pipe_operator<Parent>::value>* = nullptr>
inline Inspect<Parent, Args>
operator | (Parent&& parent, Args&& args) {
  return {std::forward<Parent>(parent), std::forward<Args>(args)};
}
} // namespace coll

