#pragma once

#include "base.hpp"

namespace coll {
template<typename Action>
struct ActArgs {
  constexpr static std::string_view name = "act";
  Action action;
};

template<typename Action>
inline ActArgs<Action> act(Action action) {
  return {std::forward<Action>(action)};
}

inline auto act() {
  return act([](auto&&){});
}

template<typename Args, typename Input>
struct ActProc {
  ActProc(const Args& args):
    args(args) {
  }

  Args args;
  auto_val(control, default_control());

  inline void process(Input e) {
    args.action(std::forward<Input>(e));
  }

  inline void end() {}

  // to be invoked by the source operator
  template<typename Exec, typename ... ArgT>
  static void execution(ArgT&& ... args) {
    auto exec = Exec(std::forward<ArgT>(args)...);
    exec.process();
    exec.end();
  };
};

template<typename Parent, typename Args,
  std::enable_if_t<Args::name == "act">* = nullptr,
  std::enable_if_t<traits::is_pipe_operator<Parent>::value>* = nullptr>
inline decltype(auto) operator | (Parent&& parent, Args&& args) {
  using Input = typename traits::remove_cvr_t<Parent>::OutputType;
  return parent.template wrap<ActProc<Args, Input>, Args&>(args);
}
} // namespace coll
