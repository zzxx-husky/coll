#pragma once

#include "base.hpp"

namespace coll {
template<typename AggregatorBuilder, typename AggregateTo>
struct AggregateArgs {
  constexpr static std::string_view name = "aggregate";

  AggregatorBuilder builder;
  // [](auto& aggregator, auto&& value) -> void {
  //   To update aggregator with value;
  // }
  AggregateTo aggregate;

  // used by operator
  template<typename Input>
  using AggregatorType = decltype(
    std::declval<AggregateArgs<AggregatorBuilder, AggregateTo>&>()
      .template get_aggregator<Input>()
  );

  template<typename Input, typename Elem = traits::remove_cvr_t<Input>>
  inline decltype(auto) get_aggregator() {
    if constexpr (traits::is_builder<AggregatorBuilder, Elem>::value) {
      return builder(Type<Elem>{});
    } else {
      return builder;
    }
  }
};

template<typename Parent, typename Args>
struct Aggregate {
  using InputType = typename traits::remove_cvr_t<Parent>::OutputType;
  using AggregatorType = typename Args::template AggregatorType<InputType>;

  Parent parent;
  Args args;

  struct AggregateProc {
    Args args;
    AggregatorType aggregator = args.template get_aggregator<InputType>();
    auto_val(control, default_control());

    AggregateProc(const Args& args): args(args) {}

    inline void process(InputType e) {
      args.aggregate(aggregator, std::forward<InputType>(e));
    }

    inline void end() {}

    inline auto result() {
      return decltype(aggregator)(std::move(aggregator));
    }

    constexpr static ExecutionType execution_type = RunExecution;

    template<typename Exec, typename ... ArgT>
    static auto execution(ArgT&& ... args) {
      auto exec = Exec(std::forward<ArgT>(args)...);
      exec.process();
      exec.end();
      return exec.result();
    }
  };

  inline decltype(auto) aggregate() {
    return parent.template wrap<AggregateProc, Args&>(args);
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

template<typename Parent, typename Args,
  std::enable_if_t<Args::name == "aggregate">* = nullptr,
  std::enable_if_t<traits::is_coll_operator<Parent>::value>* = nullptr>
inline decltype(auto) operator | (Parent&& parent, Args&& args) {
  return Aggregate<Parent, Args>{
    std::forward<Parent>(parent), std::forward<Args>(args)
  }.aggregate();
}
} // namespace coll
