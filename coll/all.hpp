#pragma once

#include "base.hpp"

namespace coll {
struct AllArgsTag {};

template<typename Condition>
struct AllArgs {
  using TagType = AllArgsTag;

  Condition condition;
};

template<typename C>
inline auto all(C&& cond) {
  return AllArgs<traits::remove_cvr_t<C>>{std::forward<C>(cond)};
}

template<typename Parent, typename Args>
struct All {
  using InputType = typename traits::remove_cvr_t<Parent>::OutputType;
  using ResultType = bool;

  Parent parent;
  Args args;

  struct Execution : public ExecutionBase {
    Args& args;
    auto_val(control, default_control());
    bool res = true;

    Execution(Args& args): args(args) {}

    inline void start() {}

    inline void process(InputType e) {
      if (!args.condition(std::forward<InputType>(e))) {
        res = false;
        control.break_now = true;
      }
    }

    inline void end() {}

    inline auto result() { return res; }

    constexpr static ExecutionType execution_type = Run;

    template<typename Exec, typename ... ArgT>
    static auto execute(ArgT&& ... args) {
      auto exec = Exec(std::forward<ArgT>(args)...);
      exec.start();
      exec.process();
      exec.end();
      return exec.result();
    }
  };

  inline decltype(auto) all() {
    return parent.template wrap<Execution>(args);
  }
};

template<typename Parent, typename Args,
  typename P = traits::remove_cvr_t<Parent>,
  typename A = traits::remove_cvr_t<Args>,
  std::enable_if_t<std::is_same<typename A::TagType, AllArgsTag>::value>* = nullptr,
  std::enable_if_t<traits::is_pipe_operator<P>::value>* = nullptr>
inline decltype(auto) operator | (Parent&& parent, Args&& args) {
  return All<P, A>{std::forward<Parent>(parent), std::forward<Args>(args)}.all();
}
} // namespace coll
