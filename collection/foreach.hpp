#pragma once

#include "base.hpp"

namespace coll {
template<typename Action>
struct ForeachArgs {
  constexpr static std::string_view name = "foreach";
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
  auto_val(control, default_control());

  inline void process(Input e) {
    args.action(std::forward<Input>(e));
  }

  inline void end() {}

  constexpr static ExecutionType execution_type = Run;

  template<typename Exec, typename ... ArgT>
  static void execute(ArgT&& ... args) {
    auto exec = Exec(std::forward<ArgT>(args)...);
    exec.process();
    exec.end();
  };
};

template<typename Parent, typename Args,
  std::enable_if_t<Args::name == "foreach">* = nullptr,
  std::enable_if_t<traits::is_pipe_operator<Parent>::value>* = nullptr>
inline decltype(auto) operator | (Parent&& parent, Args&& args) {
  using Input = typename traits::remove_cvr_t<Parent>::OutputType;
  return parent.template wrap<ForeachExecution<Args, Input>, Args&>(args);
}
} // namespace coll
