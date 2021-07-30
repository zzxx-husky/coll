#pragma once

#include "base.hpp"

namespace coll {
struct AggregateArgsTag {};

template<typename AggregatorBuilder, typename AggregateTo>
struct AggregateArgs {
  using TagType = AggregateArgsTag;

  AggregatorBuilder builder;
  // [](auto& aggregator, auto&& value) -> void {
  //   To update aggregator with value;
  // }
  AggregateTo aggregate;

  // used by operator
  template<typename Input, typename Elem = traits::remove_cvr_t<Input>>
  inline decltype(auto) get_aggregator() {
    if constexpr (traits::is_builder<AggregatorBuilder, Elem>::value) {
      return builder(Type<Elem>{});
    } else {
      return builder;
    }
  }
};

template<typename AggregatorBuilder, typename AggregateTo>
inline AggregateArgs<AggregatorBuilder, AggregateTo>
aggregate(AggregatorBuilder&& a, AggregateTo&& b) {
  return {std::forward<AggregatorBuilder>(a), std::forward<AggregateTo>(b)};
};

template<template <typename ...> class AggregatorT, typename AggregateTo>
inline auto aggregate(AggregateTo&& b) {
  return aggregate([](auto&& type) {
    return AggregatorT<typename decltype(type)::type>{};
  }, std::forward<AggregateTo>(b));
}

template<typename Parent, typename Args>
struct Aggregate {
  using InputType = typename traits::remove_cvr_t<Parent>::OutputType;
  using AggregatorType = decltype(std::declval<Args&>().template get_aggregator<InputType>());

  Parent parent;
  Args args;

  struct Execution : public ExecutionBase {
    Args args;
    // dont use auto_val here because AggregatorType might be a reference
    AggregatorType aggregator = args.template get_aggregator<InputType>();
    auto_val(control, default_control());

    Execution(const Args& args): args(args) {}

    inline void process(InputType e) {
      args.aggregate(aggregator, std::forward<InputType>(e));
    }

    inline void end() {}

    inline decltype(auto) result() {
      return decltype(aggregator)(std::move(aggregator));
    }

    constexpr static ExecutionType execution_type = Run;

    template<typename Exec, typename ... ArgT>
    static auto execute(ArgT&& ... args) {
      auto exec = Exec(std::forward<ArgT>(args)...);
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
  std::enable_if_t<std::is_same<typename A::TagType, AggregateArgsTag>::value>* = nullptr,
  std::enable_if_t<traits::is_pipe_operator<P>::value>* = nullptr>
inline decltype(auto) operator | (Parent&& parent, Args&& args) {
  return Aggregate<P, A>{
    std::forward<Parent>(parent), std::forward<Args>(args)
  }.execute();
}
} // namespace coll
