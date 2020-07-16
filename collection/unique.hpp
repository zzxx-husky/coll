#pragma once

#include "base.hpp"
#include "reference.hpp"
#include "traits.hpp"

namespace coll {
template<typename Mapper = typename Identity::type>
struct UniqueArgs {
  Mapper mapper = Identity::value;

  template<typename AnotherMapper>
  inline UniqueArgs<AnotherMapper> by(AnotherMapper m) {
    return {std::forward<AnotherMapper>(m)};
  }

  template<typename Input,
    typename MapperResult = typename traits::invocation<Mapper, Input&>::result_t>
  using ElemType = std::optional<MapperResult>;
};

template<typename Parent, typename Args>
struct Unique {
  using InputType = typename Parent::OutputType;
  using OutputType = InputType;

  Parent parent;
  Args args;

  template<typename Ctrl, typename ChildProc>
  inline void foreach(Ctrl& ctrl, ChildProc proc) {
    using ElemType = typename Args::template ElemType<InputType>;
    ElemType pre_elem; // ref or opt
    parent.foreach(ctrl,
      [&](InputType elem) {
        auto&& cur_elem = args.mapper(elem);
        auto unique = !pre_elem || *pre_elem != cur_elem;
        pre_elem = cur_elem;
        if (unique) {
          proc(std::forward<InputType>(elem));
        }
      });
  }
};

inline UniqueArgs<> unique() { return {}; }

template<typename Parent, typename Mapper,
  std::enable_if_t<traits::is_collection<Parent>::value>* = nullptr>
inline Unique<Parent, UniqueArgs<Mapper>>
operator | (const Parent& parent, UniqueArgs<Mapper> args) {
  return {parent, std::move(args)};
}
} // namespace coll

