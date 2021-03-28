#pragma once

#include "base.hpp"

namespace coll {
template<typename Input>
struct PlaceHolder {
  using OutputType = Input;

  template<typename Child, typename ... X>
  inline decltype(auto) wrap(X&& ... x) {
    if constexpr (Child::execution_type == Construct) {
      return Child::template execution<Child>(std::forward<X>(x)...);
    } else {
      return Child(std::forward<X>(x)...);
    }
  }
};

template<typename Input>
inline PlaceHolder<Input> place_holder() { return {}; }

template<typename Parent, typename ChildExecution>
struct PostPlaceHolder {
  using OutputType = typename Parent::OutputType;

  Parent parent;
  ChildExecution child_exec;

  struct Execution : public ExecutionBase {
    ChildExecution child_exec;
    auto_ref(control, child_exec.control);

    Execution(const ChildExecution& e): child_exec(e) {}

    inline void process(OutputType e) {
      child_exec.process(std::forward<OutputType>(e));
    }

    inline void end() {
      child_exec.end();
    }

    inline decltype(auto) result() {
      if constexpr (traits::execution_has_result<ChildExecution>::value) {
        return child_exec.result();
      }
    }

    constexpr static ExecutionType execution_type = Run;

    template<typename SrcExec, typename ... ArgT>
    static decltype(auto) execution(ArgT&& ... args) {
      SrcExec src_exec{std::forward<ArgT>(args)...};
      src_exec.process();
      src_exec.end();
      return src_exec.result();
    }
  };

  inline decltype(auto) execute() {
    return parent.template wrap<Execution, ChildExecution&>(child_exec);
  }
};

template<typename Parent, typename ChildExecution,
  typename P = traits::remove_cvr_t<Parent>,
  std::enable_if_t<traits::is_pipe_operator<P>::value>* = nullptr,
  std::enable_if_t<traits::is_execution<ChildExecution>::value>* = nullptr>
inline decltype(auto) operator | (Parent&& parent, ChildExecution&& exec) {
  return PostPlaceHolder<P, ChildExecution>{
    std::forward<Parent>(parent),
    std::forward<ChildExecution>(exec)
  }.execute();
}
} // namespace coll
