#pragma once

#include <utility>

#include "base.hpp"

namespace coll {
template<typename Parent, typename Args>
struct GroupByAdjacent {
  using InputType = typename Parent::OutputType;
  using KeyType = typename Args::template KeyType<InputType>;
  // using ValueType = typename Args::template ValueType<InputType>;
  using AggregatorType = typename Args::template AggregatorType<InputType>;
  using OutputType = std::pair<KeyType, AggregatorType>;

  Parent parent;
  Args args;

  template<typename Ctrl, typename ChildProc>
  void foreach(Ctrl& ctrl, ChildProc proc) {
    static_assert(!Ctrl::is_reversed, "GroupByAdjacent does not support reverse iteration. "
      "Consider to use `with_buffer()` for the closest downstream `reverse()` operator.");

    auto aggregator = args.template get_aggregator<InputType>();
    std::optional<KeyType> current_key;
    parent.foreach(ctrl,
      [&](InputType elem) {
        auto&& key = args.keyby(elem);
        if (key != current_key) {
          if (bool(current_key)) {
            proc(OutputType{*current_key, std::move(aggregator)});
            aggregator = std::move(args.template get_aggregator<InputType>());
            // TODO(zzxx):
            // if constexpr (traits::has_clear<AggregatorType>::value) {
            //   aggregator.clear();
            // } else {
            //   aggregator = std::move(args.template get_aggregator<InputType>());
            // }
          }
          current_key = key;
        }
        args.aggregate_to(aggregator, args.valby(elem));
      });
    if (bool(current_key)) {
      proc(OutputType{*current_key, std::move(aggregator)});
    }
  }
};
} // namespace coll
