#pragma once

#include "base.hpp"

namespace coll {
template<typename P>
struct ForeachArgs {
  constexpr static std::string_view name = "foreach";
  P process;
};

template<typename Parent, typename Args>
struct Foreach {
  using InputType = typename traits::remove_cvr_t<Parent>::OutputType;
  using OutputType = InputType;

  Parent parent;
  Args args;

  template<typename Child>
  struct Proc : public Child {
    Args args;

    template<typename ... X>
    Proc(const Args& args, X&& ... x):
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
    return parent.template wrap<Proc<Child>, Args&, X...>(
      args, std::forward<X>(x)...
    );
  }
};

template<typename P>
ForeachArgs<P> foreach(P process) { return {std::forward<P>(process)}; }

template<typename Parent, typename Args,
  std::enable_if_t<Args::name == "foreach">* = nullptr,
  std::enable_if_t<traits::is_coll_operator<Parent>::value>* = nullptr>
inline Foreach<Parent, Args>
operator | (Parent&& parent, Args&& args) {
  return {std::forward<Parent>(parent), std::forward<Args>(args)};
}
} // namespace coll

