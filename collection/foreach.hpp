#pragma once

#include "base.hpp"

namespace coll {
template<typename P>
struct ForeachArgs {
  P process;
};

template<typename Parent, typename P>
struct Foreach {
  using InputType = typename Parent::OutputType;
  using OutputType = InputType;

  Parent parent;
  ForeachArgs<P> args;

  template<typename Ctrl, typename ChildProc>
  inline void foreach(Ctrl& ctrl, ChildProc proc) {
    parent.foreach(ctrl,
      [&](InputType elem) {
        args.process(std::forward<InputType>(elem));
        proc(std::forward<InputType>(elem));
      });
  }
};

template<typename P>
ForeachArgs<P> foreach(P process) { return {std::forward<P>(process)}; }

template<typename Parent, typename P,
  std::enable_if_t<traits::is_collection<Parent>::value>* = nullptr>
inline Foreach<Parent, P> operator | (const Parent& parent, ForeachArgs<P> args) {
  return {parent, std::move(args)};
}
} // namespace coll

