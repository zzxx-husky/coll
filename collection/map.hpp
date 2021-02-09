#pragma once

#include "base.hpp"

namespace coll {
template<typename M>
struct MapArgs {
  constexpr static std::string_view name = "map";

  template<typename Input>
  using MapperResultType = typename traits::invocation<M, Input>::result_t;

  M mapper;
};

template<typename Parent, typename Args>
struct Map {
  using InputType = typename traits::remove_cvr_t<Parent>::OutputType;
  using OutputType = typename Args::template MapperResultType<InputType>;

  Parent parent;
  Args args;

  template<typename Child>
  struct Proc : public Child {
    Args args;

    template<typename ... X>
    Proc(const Args& args, X&& ... x):
      args(args),
      Child(std::forward<X>(x)...) {
    }

    inline void process(InputType e) {
      Child::process(args.mapper(std::forward<InputType>(e)));
    }
  };

  template<typename Child, typename ... X>
  inline decltype(auto) wrap(X&& ... x) {
    return parent.template wrap<Proc<Child>, Args&, X...>(
      args, std::forward<X>(x) ...
    );
  }
};

template<typename M>
MapArgs<M> map(M mapper) { return {mapper}; }

template<typename Parent, typename Args,
  std::enable_if_t<Args::name == "map">* = nullptr,
  std::enable_if_t<traits::is_coll_operator<Parent>::value>* = nullptr>
inline Map<Parent, Args>
operator | (Parent&& parent, Args&& args) {
  return {std::forward<Parent>(parent), std::forward<Args>(args)};
}
} // namespace coll
