#pragma once

#include "base.hpp"

namespace coll {
struct ToArgsTag {};

template<typename ContainerBuilder, bool CopyOrMove>
struct ToArgs {
  using TagType = ToArgsTag;

  // 1. a container copy;
  // 2. a reference to an existing container
  // 3. a builder that constructs a new container
  ContainerBuilder builder;

  // used by users
  inline ToArgs<ContainerBuilder, false> by_move() {
    return {std::forward<ContainerBuilder>(builder)};
  }

  // used by operators
  constexpr static bool ins_by_copy = CopyOrMove;
  constexpr static bool ins_by_move = !CopyOrMove;

  constexpr static bool is_container_ref =
    // Note: no 'const T&' appear here, only T&
    std::is_lvalue_reference<ContainerBuilder>::value;

  template<typename Input, typename Elem = traits::remove_cvr_t<Input>>
  decltype(auto) get_container() {
    if constexpr (traits::is_builder<ContainerBuilder, Elem>::value) {
      // Builder
      return builder(Type<Elem>{});
    } else {
      // Reference or Value
      return builder;
    }
  }
};

template<typename ContainerVB>
inline ToArgs<ContainerVB, true> to(ContainerVB&& vb) {
  // std::move to handle const T&
  return {std::move(vb)};
}

template<typename ContainerR>
inline ToArgs<ContainerR&, true> to(ContainerR& container_ref) {
  return {container_ref};
}

template<typename ContainerV>
inline auto to() {
  return to([=](auto&&) { return ContainerV{}; });
}

template<template<typename ...> class ContainerT>
inline auto to() {
  return to([](auto type) {
    return ContainerT<typename decltype(type)::type>();
  });
}

template<typename Parent, typename Args>
struct To {
  using InputType = typename Parent::OutputType;

  Parent parent;
  Args args;

  struct Execution : public ExecutionBase {
    Execution(const Args& args): args(args) {}

    Args args;
    auto_val(container, args.template get_container<InputType>());
    auto_val(control,   default_control());

    inline void start() {}

    inline void process(InputType e) {
      if constexpr (Args::ins_by_copy) {
        container_utils::insert(container, std::forward<InputType>(e));
      } else {
        container_utils::insert(container, std::move(e));
      }
    }

    inline void end() {}

    inline decltype(auto) result() {
      if constexpr (Args::is_container_ref) {
        return container;
      } else {
        // make a new copy, move existing member to the copy, return the copy
        // the new copy will be constructed in-place.
        return decltype(container)(std::move(container));
      }
    }

    constexpr static ExecutionType execution_type = Run;

    template<typename Exec, typename ... ArgT>
    static decltype(auto) execute(ArgT&& ... args) {
      auto exec = Exec(std::forward<ArgT>(args)...);
      exec.start();
      exec.process();
      exec.end();
      return exec.result();
    }
  };

  inline decltype(auto) execute() {
    return parent.template wrap<Execution, Args&>(args);
  }
};

template<typename Parent, typename Args,
  typename P = traits::remove_cvr_t<Parent>,
  typename A = traits::remove_cvr_t<Args>,
  std::enable_if_t<std::is_same<typename A::TagType, ToArgsTag>::value>* = nullptr,
  std::enable_if_t<traits::is_pipe_operator<P>::value>* = nullptr>
inline decltype(auto)
operator | (Parent&& parent, Args&& args) {
  return To<P, A>{std::forward<Parent>(parent), std::forward<Args>(args)}.execute();
}
} // namespace coll
