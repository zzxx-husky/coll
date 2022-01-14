#pragma once

#include "base.hpp"
#include "container_utils.hpp"

namespace coll {
struct SplitArgsTag {};

template<
  typename Condition,
  typename ContainerBuilder,
  bool CacheByRef = false>
struct SplitArgs {
  using TagType = SplitArgsTag;
  // Container is expected to have the following member functions:
  // 1. insert / push_back / push / emplace for insertion
  // 2. clear
  ContainerBuilder container_builder;
  Condition condition;

  // used by user
  inline SplitArgs<Condition, ContainerBuilder, CacheByRef>
  cache_by_ref() { return {}; }

  // used by operator
  template<typename Input, typename Elem = traits::remove_cvr_t<Input>>
  inline decltype(auto) make_container() {
    if constexpr (traits::is_builder<ContainerBuilder, Elem>::value) {
      return container_builder(Type<Elem>{});
    } else {
      return container_builder;
    }
  }
};

template<typename Parent, typename Args>
struct Split {
  using InputType = typename Parent::OutputType;
  using OutputType = decltype(std::declval<Args>().template make_container<InputType>());

  Parent parent;
  Args args;

  template<typename Child>
  struct Execution : public Child {
    Args args;
    auto_val(container, args.template make_container<InputType>());
    auto_val(is_empty,  true);

    template<typename ...X>
    Execution(const Args& args, X&& ... x):
      args(args),
      Child(std::forward<X>(x)...) {
    }

    inline void process(InputType e) {
      if (args.condition(e)) {
        if (likely(!is_empty)) {
          Child::process(container);
          container.clear();
          is_empty = true;
        }
      } else {
        container_utils::insert(container, std::forward<InputType>(e));         
        is_empty = false;
      }
    }

    inline void end() {
      if (!is_empty) {
        Child::process(container);
        container.clear();
      }
      Child::end();
    }
  };

  template<ExecutionType ET, typename Child, typename ... X>
  inline decltype(auto) wrap(X&& ... x) {
    using Ctrl = traits::operator_control_t<Child>;
    static_assert(!Ctrl::is_reversed, "Spilt does not support reverse iteration. "
      "Consider to use `with_buffer()` for the closest downstream `reverse()` operator.");
    
    return parent.template wrap<ET, Execution<Child>, Args&, X...>(
      args, std::forward<X>(x)...
    );
  }
};

template<typename Container, typename Condition>
inline SplitArgs<Condition, Container, false>
split(Container&& container, Condition condition) {
  return {std::forward<Container>(container), std::forward<Condition>(condition)};
}

template<template<typename ...> class ContainerTemplate, typename Condition>
inline auto split(Condition condition) {
  return split([](auto&& type) {
    return ContainerTemplate<typename decltype(type)::type>{};
  }, std::forward<Condition>(condition));
}

template<typename Condition>
inline auto split(Condition condition) {
  return split<std::vector>(std::forward<Condition>(condition));
}

template<typename Parent, typename Args,
  typename P = traits::remove_cvr_t<Parent>,
  typename A = traits::remove_cvr_t<Args>,
  std::enable_if_t<std::is_same<typename A::TagType, SplitArgsTag>::value>* = nullptr,
  std::enable_if_t<traits::is_pipe_operator<P>::value>* = nullptr>
inline Split<P, A>
operator | (Parent&& parent, Args&& args) {
  return {std::forward<Parent>(parent), std::forward<Args>(args)};
}
} // namespace coll
