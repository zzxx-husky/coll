#pragma once

#include "base.hpp"

namespace coll {

template<typename ParentColl, typename ChildColl>
struct Link {
  using InputType = typename ParentColl::OutputType;
  using OutputType = typename ChildColl::OutputType;

  ParentColl parent_coll;
  ChildColl child_coll;

  template<typename ChildExec>
  struct Execution : public ExecutionBase {
    Execution(ChildExec&& exec):
      child_exec(exec) {
    }

    ChildExec child_exec;
    auto_ref(control, child_exec.control);

    inline void start() { child_exec.start(); }

    inline void process(InputType e) {
      child_exec.process(std::forward<InputType>(e));
    }

    inline void end() { child_exec.end(); }

    inline decltype(auto) result() { return child_exec.result(); }

    constexpr static ExecutionType execution_type = Run;

    template<typename Exec, typename ... ArgT>
    static decltype(auto) execute(ArgT&& ... args) {
      auto exec = Exec(std::forward<ArgT>(args)...);
      exec.start();
      exec.process();
      exec.end();
      return exec.result();
    }
  };

  template<typename Child, typename ... X>
  inline decltype(auto) wrap(X&& ... x) {
    auto child_exec = child_coll.template wrap<Child, X...>(std::forward<X>(x) ...);
    using T = decltype(child_exec);
    return parent_coll.template wrap<Execution<T>>(std::forward<T>(child_exec));
  }
};

template<typename ParentColl, typename ChildColl,
  typename P = traits::remove_cvr_t<ParentColl>,
  typename C = traits::remove_cvr_t<ChildColl>,
  std::enable_if_t<traits::is_pipe_operator<P>::value>* = nullptr,
  std::enable_if_t<traits::is_pipe_operator<C>::value>* = nullptr>
inline Link<P, C> operator | (ParentColl&& parent, ChildColl&& child) {
  return {std::forward<ParentColl>(parent), std::forward<ChildColl>(child)};
}
} // namespace coll
