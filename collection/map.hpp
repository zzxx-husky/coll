#pragma once

#include "base.hpp"

namespace coll {
template<typename M>
struct MapArgs {
  M mapper;
};

template<typename Parent, typename M>
struct Map {
  using InputType = typename Parent::OutputType;
  using OutputType = typename traits::invocation<M, InputType>::result_t;

  Parent parent;
  MapArgs<M> args;

  template<typename Ctrl, typename ChildProc>
  inline void foreach(Ctrl& ctrl, ChildProc proc) {
    parent.foreach(ctrl,
      [&](InputType elem) {
        proc(args.mapper(std::forward<InputType>(elem)));
      });
  }
};

template<typename M>
MapArgs<M> map(M mapper) { return {mapper}; }

template<typename Parent, typename M,
  std::enable_if_t<traits::is_collection<Parent>::value>* = nullptr>
inline Map<Parent, M> operator | (const Parent& parent, MapArgs<M> args) {
  return {parent, std::move(args)};
}
} // namespace coll
