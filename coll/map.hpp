#pragma once

#include "base.hpp"
#include "traits.hpp"

namespace coll {
struct MapArgsTag {};

template<typename M>
struct MapArgs {
  using TagType = MapArgsTag;

  template<typename Input>
  using MapperResultType = typename traits::invocation<M, Input>::result_t;

  M mapper;
};

template<typename M>
MapArgs<M> map(M mapper) { return {mapper}; }

template<typename Parent, typename Args>
struct Map {
  using InputType = typename traits::remove_cvr_t<Parent>::OutputType;
  using OutputType = typename Args::template MapperResultType<InputType>;

  Parent parent;
  Args args;

  template<typename Child>
  struct Execution : public Child {
    Args args;

    template<typename ... X>
    Execution(const Args& args, X&& ... x):
      args(args),
      Child(std::forward<X>(x)...) {
    }

    inline void process(InputType e) {
      Child::process(args.mapper(std::forward<InputType>(e)));
    }
  };

  template<ExecutionType ET, typename Child, typename ... X>
  inline decltype(auto) wrap(X&& ... x) {
    return parent.template wrap<ET, Execution<Child>>(
      args, std::forward<X>(x) ...
    );
  }
};

template<typename Parent, typename Args,
  typename P = traits::remove_cvr_t<Parent>,
  typename A = traits::remove_cvr_t<Args>,
  std::enable_if_t<std::is_same<typename A::TagType, MapArgsTag>::value>* = nullptr,
  std::enable_if_t<traits::is_pipe_operator<P>::value>* = nullptr>
inline Map<P, A>
operator | (Parent&& parent, Args&& args) {
  return {std::forward<Parent>(parent), std::forward<Args>(args)};
}

template<typename Optional, typename Args,
  typename O = traits::remove_cvr_t<Optional>,
  typename A = traits::remove_cvr_t<Args>,
  std::enable_if_t<std::is_same<typename A::TagType, MapArgsTag>::value>* = nullptr,
  std::enable_if_t<traits::is_optional<O>::value>* = nullptr>
inline auto operator | (Optional&& optional, Args&& args) {
  return bool(optional)
    ? std::optional(args.mapper(*optional))
    : std::nullopt;
}
} // namespace coll
