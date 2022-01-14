#pragma once

#include "base.hpp"

namespace coll {
template<typename Input>
struct PlaceHolder {
  using OutputType = Input;

  template<ExecutionType ET, typename Child, typename ... X>
  inline decltype(auto) wrap(X&& ... x) {
    if constexpr (ET == Construct) {
      // call `execute` because the child knows how to construct
      return Child::template construct<ExecutionType::Object, Child>(std::forward<X>(x)...);
    } else /* if constexpr (ET == Execute || ET == Object) */ {
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

    Execution(const ChildExecution& e): child_exec(e) {}

    inline auto& control() {
      return child_exec.control();
    }

    inline void start() {
      child_exec.start();
    }

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

    template<typename SrcExec, typename ... ArgT>
    static decltype(auto) execute(ArgT&& ... args) {
      SrcExec src_exec{std::forward<ArgT>(args)...};
      src_exec.process();
      src_exec.end();
      return src_exec.result();
    }
  };

  inline decltype(auto) execute() {
    return parent.template wrap<ExecutionType::Execute, Execution, ChildExecution&>(child_exec);
  }
};

template<typename Parent, typename ChildExecution,
  typename P = traits::remove_cvr_t<Parent>,
  typename C = traits::remove_cvr_t<ChildExecution>,
  std::enable_if_t<traits::is_pipe_operator<P>::value>* = nullptr,
  std::enable_if_t<traits::is_execution<C>::value>* = nullptr>
inline decltype(auto) operator | (Parent&& parent, ChildExecution&& exec) {
  return PostPlaceHolder<P, C>{
    std::forward<Parent>(parent),
    std::forward<ChildExecution>(exec)
  }.execute();
}
} // namespace coll
