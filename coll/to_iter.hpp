#pragma once

#include "base.hpp"

namespace coll {
struct ToIterArgsTag {};

template<typename Iter, bool CopyOrMove>
struct ToIterArgs {
  using TagType = ToIterArgsTag;

  Iter start;
  std::optional<Iter> end = std::nullopt;

  // used by users
  inline ToIterArgs<Iter, false> by_move() {
    return {std::forward<Iter>(start), end};
  }

  // used by operators
  constexpr static bool ins_by_copy = CopyOrMove;
  constexpr static bool ins_by_move = !CopyOrMove;
};

template<typename Iter,
  typename I = traits::remove_cvr_t<Iter>,
  std::enable_if_t<traits::is_iterator<I>::value>* = nullptr>
inline ToIterArgs<I, true> to_iter(Iter&& start) {
  return {std::forward<Iter>(start)};
}

template<typename Iter,
  typename I = traits::remove_cvr_t<Iter>,
  std::enable_if_t<traits::is_iterator<I>::value>* = nullptr>
inline ToIterArgs<I, true> to_iter(Iter&& start, Iter&& end) {
  return {std::forward<Iter>(start), {std::forward<Iter>(end)}};
}

template<typename Parent, typename Args>
struct ToIter {
  using InputType = typename Parent::OutputType;

  Parent parent;
  Args args;

  struct Execution : public ExecutionBase {
    Execution(const Args& args): args(args) {}

    Args args;
    auto_val(ctrl, default_control());

    inline auto& control() {
      return ctrl;
    }

    inline void start() {
      if (args.start == args.end) {
        this->ctrl.break_now = true;
      }
    }

    inline void process(InputType e) {
      if constexpr (Args::ins_by_copy) {
        *args.start = std::forward<InputType>(e);
      } else {
        *args.start = std::move(e);
      }
      ++args.start;
      if (args.start == args.end) {
        this->ctrl.break_now = true;
      }
    }

    inline void end() {}

    inline decltype(auto) result() {
      return args.start;
    }

    template<typename Exec, typename ... ArgT>
    static decltype(auto) execute(ArgT&& ... args) {
      auto exec = Exec(std::forward<ArgT>(args)...);
      exec.start();
      exec.run();
      exec.end();
      return exec.result();
    }
  };

  inline decltype(auto) execute() {
    return parent.template wrap<ExecutionType::Execute, Execution, Args&>(args);
  }
};

template<typename Parent, typename Args,
  typename P = traits::remove_cvr_t<Parent>,
  typename A = traits::remove_cvr_t<Args>,
  std::enable_if_t<std::is_same<typename A::TagType, ToIterArgsTag>::value>* = nullptr,
  std::enable_if_t<traits::is_pipe_operator<P>::value>* = nullptr>
inline decltype(auto)
operator | (Parent&& parent, Args&& args) {
  return ToIter<P, A>{std::forward<Parent>(parent), std::forward<Args>(args)}.execute();
}
} // namespace coll

