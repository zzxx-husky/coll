#pragma once

#include "base.hpp"

namespace coll {
template<typename F>
struct FilterArgs {
  F filter;
};

template<typename Parent, typename F>
struct Filter {
  using InputType = typename Parent::OutputType;
  using OutputType = InputType;

  Parent parent;
  FilterArgs<F> args;

  template<typename Ctrl, typename ChildProc>
  inline void foreach(Ctrl& ctrl, ChildProc proc) {
    parent.foreach(ctrl,
      [=](InputType elem) {
        if (args.filter(elem)) {
          proc(std::forward<InputType>(elem));
        }
      });
  }
};

template<typename F>
inline FilterArgs<F> filter(F lambda) { return {lambda}; }

template<typename Parent, typename F,
  std::enable_if_t<traits::is_collection<Parent>::value>* = nullptr>
inline Filter<Parent, F> operator | (const Parent& parent, FilterArgs<F> args) {
  return {parent, std::move(args)};
}
} // namespace coll
