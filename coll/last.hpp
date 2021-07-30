#pragma once

#include "base.hpp"
#include "reference.hpp"

namespace coll {
struct LastArgsTag {};

template<bool Ref>
struct LastArgs {
  using TagType = LastArgsTag;

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

  struct Execution : public ExecutionBase {
    ResultType res;
    auto_val(control, default_control());

    inline void process(InputType e) {
      res = std::forward<InputType>(e);
    }

    inline void end() {}

    inline auto& result() { return res; }

    constexpr static ExecutionType execution_type = Run;

    template<typename Exec, typename ... ArgT>
    static auto execute(ArgT&& ... args) {
      auto exec = Exec(std::forward<ArgT>(args)...);
      exec.process();
      exec.end();
      return std::move(exec.result());
    }
  };

  inline decltype(auto) last() {
    return parent.template wrap<Execution>();
  }
};

inline LastArgs<false> last() { return {}; }

template<typename Parent, typename Args,
  typename P = traits::remove_cvr_t<Parent>,
  typename A = traits::remove_cvr_t<Args>,
  std::enable_if_t<std::is_same<typename A::TagType, LastArgsTag>::value>* = nullptr,
  std::enable_if_t<traits::is_pipe_operator<P>::value>* = nullptr>
inline decltype(auto) operator | (Parent&& parent, Args&& args) {
  return Last<P, A>{std::forward<Parent>(parent), std::forward<Args>(args)}.last();
}
} // namespace coll
