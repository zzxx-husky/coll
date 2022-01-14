#pragma once

#include "base.hpp"
#include "reference.hpp"

namespace coll {
struct HeadArgsTag {};

template<bool Ref>
struct HeadArgs {
  using TagType = HeadArgsTag;

  inline HeadArgs<true> ref() { return {}; }

  constexpr static bool use_ref = Ref;
};

inline HeadArgs<false> head() { return {}; }

template<typename Parent, typename Args>
struct Head {
  using InputType = typename traits::remove_cvr_t<Parent>::OutputType;
  using ResultType = std::conditional_t<Args::use_ref,
    Reference<typename traits::remove_vr_t<InputType>>,
    std::optional<typename traits::remove_cvr_t<InputType>>
  >;

  Parent parent;
  Args args;

  struct Execution : public ExecutionBase {
    ResultType res;
    auto_val(ctrl, default_control());

    inline void start() {}

    inline auto& control() {
      return ctrl;
    }

    inline void process(InputType e) {
      res = std::forward<InputType>(e);
      ctrl.break_now = true;
    }

    inline void end() {}

    inline auto& result() { return res; }

    template<typename Exec, typename ... ArgT>
    static auto execute(ArgT&& ... args) {
      auto exec = Exec(std::forward<ArgT>(args)...);
      exec.start();
      exec.launch();
      exec.end();
      return std::move(exec.result());
    }
  };

  inline decltype(auto) head() {
    return parent.template wrap<ExecutionType::Execute, Execution>();
  }
};

template<typename Parent, typename Args,
  typename P = traits::remove_cvr_t<Parent>,
  typename A = traits::remove_cvr_t<Args>,
  std::enable_if_t<std::is_same<typename A::TagType, HeadArgsTag>::value>* = nullptr,
  std::enable_if_t<traits::is_pipe_operator<P>::value>* = nullptr>
inline decltype(auto) operator | (Parent&& parent, Args&& args) {
  return Head<P, A>{std::forward<Parent>(parent), std::forward<Args>(args)}.head();
}
} // namespace coll

