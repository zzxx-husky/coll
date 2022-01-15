#pragma once

#include "base.hpp"

namespace coll {
struct ForeachArgsTag {};

template<typename Action>
struct ForeachArgs {
  using TagType = ForeachArgsTag;
  Action action;
};

template<typename Action>
inline ForeachArgs<Action> foreach(Action action) {
  return {std::forward<Action>(action)};
}

inline auto act() {
  return foreach([](auto&&){});
}

template<typename Args, typename Input>
struct ForeachExecution : public ExecutionBase {
  ForeachExecution(const Args& args):
    args(args) {
  }

  Args args;
  auto_val(ctrl, default_control());

  inline auto& control() {
    return ctrl;
  }

  inline void start() {}

  inline void process(Input e) {
    args.action(std::forward<Input>(e));
  }

  inline void end() {}

  template<typename Exec, typename ... ArgT>
  static void execute(ArgT&& ... args) {
    auto exec = Exec(std::forward<ArgT>(args)...);
    exec.start();
    exec.launch();
    exec.end();
  };
};

template<typename Parent, typename Args,
  typename P = traits::remove_cvr_t<Parent>,
  typename A = traits::remove_cvr_t<Args>,
  std::enable_if_t<std::is_same<typename A::TagType, ForeachArgsTag>::value>* = nullptr,
  std::enable_if_t<traits::is_pipe_operator<P>::value>* = nullptr>
inline decltype(auto) operator | (Parent&& parent, Args&& args) {
  using Input = typename P::OutputType;
  return parent.template wrap<
    ExecutionType::Execute,
    ForeachExecution<Args, Input>,
    Args&
  >(args);
}
} // namespace coll
