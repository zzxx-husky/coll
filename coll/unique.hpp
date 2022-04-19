#pragma once

#include "base.hpp"
#include "reference.hpp"
#include "traits.hpp"

namespace coll {
struct UniqueArgsTag {};

template<typename Mapper = typename Identity::type>
struct UniqueArgs {
  using TagType = UniqueArgsTag;

  Mapper mapper = Identity::value;

  template<typename AnotherMapper>
  inline UniqueArgs<AnotherMapper> by(AnotherMapper m) {
    return {std::forward<AnotherMapper>(m)};
  }

  template<typename Input,
    typename MapperResult = typename traits::invocation<Mapper, Input&>::result_t>
  using ElemType = std::optional<MapperResult>;
};

inline UniqueArgs<> unique() { return {}; }

template<typename Parent, typename Args>
struct Unique {
  using InputType = typename Parent::OutputType;
  using OutputType = InputType;
  using ElemType = typename Args::template ElemType<InputType>;

  Parent parent;
  Args args;

  template<typename Child>
  struct Execution : public Child {
    Args args;
    ElemType pre_elem;

    template<typename ...X>
    Execution(const Args& args, X&& ... x):
      Child(std::forward<X>(x)...),
      args(args) {
    }

    inline void process(InputType e) {
      auto&& cur_elem = args.mapper(e);
      auto unique = !pre_elem || *pre_elem != cur_elem;
      pre_elem = cur_elem;
      if (unique) {
        Child::process(std::forward<InputType>(e));
      }
    }
  };

  template<ExecutionType ET, typename Child, typename ... X>
  inline decltype(auto) wrap(X&& ... x) {
    return parent.template wrap<ET, Execution<Child>>(args, std::forward<X>(x)...);
  }
};

template<typename Parent, typename Args,
  typename P = traits::remove_cvr_t<Parent>,
  typename A = traits::remove_cvr_t<Args>,
  std::enable_if_t<std::is_same<typename A::TagType, UniqueArgsTag>::value>* = nullptr,
  std::enable_if_t<traits::is_pipe_operator<P>::value>* = nullptr>
inline Unique<P, A>
operator | (Parent&& parent, Args&& args) {
  return {std::forward<Parent>(parent), std::forward<Args>(args)};
}
} // namespace coll

