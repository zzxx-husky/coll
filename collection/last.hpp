#pragma once

#include "base.hpp"
#include "reference.hpp"

namespace coll {
template<bool Ref>
struct LastArgs {
  constexpr static std::string_view name = "last";

  inline LastArgs<true> ref() { return {}; }

  constexpr static bool use_ref = Ref;
};

template<typename Parent, typename Args>
struct Last {
  using InputType = typename traits::remove_cvr_t<Parent>::OutputType;
  using ResultType = std::conditional_t<Args::use_ref,
    Reference<typename traits::remove_vr_t<InputType>>,
    std::optional<typename traits::remove_cvr_t<InputType>>
  >;

  Parent parent;
  Args args;

  struct LastProc {
    ResultType res;
    auto_val(control, default_control());

    inline void process(InputType e) {
      res = std::forward<InputType>(e);
    }

    inline void end() {}

    inline auto& result() { return res; }

    constexpr static ExecutionType execution_type = RunExecution;
    template<typename Exec, typename ... ArgT>
    static auto execution(ArgT&& ... args) {
      auto exec = Exec(std::forward<ArgT>(args)...);
      exec.process();
      exec.end();
      return std::move(exec.result());
    }
  };

  inline decltype(auto) last() {
    return parent.template wrap<LastProc>();
  }
};

inline LastArgs<false> last() { return {}; }

template<typename Parent, typename Args,
  std::enable_if_t<Args::name == "last">* = nullptr,
  std::enable_if_t<traits::is_pipe_operator<Parent>::value>* = nullptr>
inline decltype(auto) operator | (Parent&& parent, Args&& args) {
  return Last<Parent, Args>{std::forward<Parent>(parent), std::forward<Args>(args)}.last();
}
} // namespace coll
