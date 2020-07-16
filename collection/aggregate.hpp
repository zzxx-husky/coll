#pragma once

#include "base.hpp"

namespace coll {
template<typename AggregatorVB, typename AggregateTo>
struct AggregateArgs {
  AggregatorVB vb;
  // [](auto& aggregator, auto&& value) -> void {
  //   To update aggregator with value;
  // }
  AggregateTo aggregate;

  template<typename Input, typename Elem = traits::remove_cvr_t<Input>>
  auto get_aggregator() {
    if constexpr (traits::is_builder<AggregatorVB, Elem>::value) {
      // Builder
      return vb(Type<Elem>{});
    } else {
      // Value
      return vb;
    }
  }
};

template<typename Parent>
struct Aggregate {
  using InputType = typename Parent::OutputType;

  Parent parent;

  template<typename Args>
  inline auto aggregate(Args args) {
    auto ctrl = default_control();
    auto aggregator = args.template get_aggregator<InputType>();
    parent.foreach(ctrl,
      [&](InputType elem) {
        args.aggregate(aggregator, std::forward<InputType>(elem));
      });
    return aggregator;
  }
};

template<typename AggregatorVB, typename AggregateTo>
inline AggregateArgs<AggregatorVB, AggregateTo>
aggregate(AggregatorVB a, AggregateTo b) {
  return {std::forward<AggregatorVB>(a), std::forward<AggregateTo>(b)};
};

template<template <typename ...> class AggregatorT, typename AggregateTo>
inline auto aggregate(AggregateTo b) {
  return aggregate([](auto&& type) {
    return AggregatorT<typename traits::remove_cvr_t<decltype(type)>::type>();
  }, std::forward<AggregateTo>(b));
}

template<typename Parent, typename Aggregator, typename AggregateTo,
  std::enable_if_t<traits::is_collection<Parent>::value>* = nullptr>
inline auto operator | (const Parent& parent, AggregateArgs<Aggregator, AggregateTo> args) {
  return Aggregate<Parent>{parent}.aggregate(std::move(args));
}
} // namespace coll
