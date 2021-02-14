#pragma once

#include "base.hpp"
#include "reference.hpp"
#include "traits.hpp"

namespace coll {
template<typename Mapper = typename Identity::type>
struct UniqueArgs {
  constexpr static std::string_view name = "unique";

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
  using ElemType = typename Args::template ElemType<InputType>;

  Parent parent;
  Args args;

  template<typename Child>
  struct Proc : public Child {
    Args args;
    ElemType pre_elem;

    template<typename ...X>
    Proc(const Args& args, X&& ... x):
      args(args),
      Child(std::forward<X>(x)...) {
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

  template<typename Child, typename ... X>
  inline decltype(auto) wrap(X&& ... x) {
    return parent.template wrap<Proc<Child>, Args&, X...>(args, std::forward<X>(x)...);
  }
};

inline UniqueArgs<> unique() { return {}; }

template<typename Parent, typename Args,
  std::enable_if_t<Args::name == "unique">* = nullptr,
  std::enable_if_t<traits::is_pipe_operator<Parent>::value>* = nullptr>
inline Unique<Parent, Args>
operator | (Parent&& parent, Args&& args) {
  return {std::forward<Parent>(parent), std::forward<Args>(args)};
}
} // namespace coll

