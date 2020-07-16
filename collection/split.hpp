#pragma once

#include "base.hpp"
#include "container_utils.hpp"

namespace coll {
template<
  typename Condition,
  template<typename ...> class ContainerTemplate = NullTemplateArg,
  typename Container = NullArg,
  bool CacheByRef = false>
struct SplitArgs {
  Condition condition;
  // Container is expected to have the following member functions:
  // 1. insert / push_back / push / emplace for insertion
  // 2. clear
  Container container;

  template<typename Input>
  using ContainerType = std::conditional_t<std::is_same<Container, NullArg>::value,
    std::conditional_t<CacheByRef,
      ContainerTemplate<Reference<traits::remove_vr_t<Input>>>,
      ContainerTemplate<traits::remove_cvr_t<Input>>
    >,
    Container
  >;

  template<typename Input>
  inline auto make_container() {
    if constexpr (std::is_same<Container, NullArg>::value) {
      return ContainerType<Input>{};
    } else {
      return container;
    }
  }

  inline SplitArgs<Condition, ContainerTemplate, Container, CacheByRef> cache_by_ref() { return {}; }
};

template<typename Parent, typename Args>
struct Split {
  using InputType = typename Parent::OutputType;
  using OutputType = typename Args::template ContainerType<InputType>&;

  Parent parent;
  Args args;

  template<typename Ctrl, typename ChildProc>
  void foreach(Ctrl& ctrl, ChildProc proc) {
    static_assert(!Ctrl::is_reversed, "Spilt does not support reverse iteration. "
      "Consider to use `with_buffer()` for the closest downstream `reverse()` operator.");

    auto container = args.template make_container<InputType>();
    bool is_empty = true;
    parent.foreach(ctrl,
      [&](InputType elem) {
        if (args.condition(elem)) {
          if (likely(!is_empty)) {
            proc(container);
            container.clear();
            is_empty = true;
          }
        } else {
          container_utils::insert(container, std::forward<InputType>(elem));         
          is_empty = false;
        }
      });
    if (!is_empty) {
      proc(container);
      container.clear();
    }
  }
};

template<template<typename ...> class ContainerTemplate, typename Condition>
inline SplitArgs<Condition, ContainerTemplate, NullArg, false>
split(Condition condition) {
  return {std::forward<Condition>(condition)};
}

template<typename Container, typename Condition>
inline SplitArgs<Condition, NullTemplateArg, Container, false>
split(Container&& container, Condition condition) {
  return {std::forward<Condition>(condition), std::forward<Container>(container)};
}

template<typename Condition>
inline auto split(Condition condition) {
  return split<std::vector>(std::forward<Condition>(condition));
}

template<typename Parent, typename Condition, template<typename ...> class ContainerTemplate,
  typename Container, bool CacheByRef,
  std::enable_if_t<traits::is_collection<Parent>::value>* = nullptr>
inline Split<Parent, SplitArgs<Condition, ContainerTemplate, Container, CacheByRef>>
operator | (const Parent& parent, SplitArgs<Condition, ContainerTemplate, Container, CacheByRef> args) {
  return {parent, std::move(args)};
}
} // namespace coll
