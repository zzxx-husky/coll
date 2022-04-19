#pragma once

#include <utility>

#include "base.hpp"

namespace coll {
template<typename Parent, typename Args>
struct GroupByAdjacent {
  using InputType = typename traits::remove_cvr_t<Parent>::OutputType;
  using KeyType = typename Args::template KeyType<InputType>;
  using AggregatorType = decltype(std::declval<Args&>().template get_aggregator<InputType>());
  using OutputType = std::pair<KeyType, AggregatorType>;

  Parent parent;
  Args args;

  template<typename Child>
  struct Execution : public Child {
    Args args;
    std::optional<KeyType> current_key;
    auto_val(aggregator, args.template get_aggregator<InputType>());

    template<typename ...X>
    Execution(const Args& args, X&& ... x):
      Child(std::forward<X>(x)...),
      args(args) {
    }

    inline void process(InputType elem) {
      auto&& key = args.keyby(elem);
      if (key != current_key) {
        if (bool(current_key)) {
          Child::process(OutputType{*current_key, std::move(aggregator)});
          aggregator = std::move(args.template get_aggregator<InputType>());
          // TODO(zzxx): how to clear the aggregator;
          // if constexpr (traits::has_clear<AggregatorType>::value) {
          //   aggregator.clear();
          // } else {
          //   aggregator = std::move(args.template get_aggregator<InputType>());
          // }
        }
        current_key = key;
      }
      args.aggregate_to(aggregator, args.valby(elem));
    }

    inline void end() {
      if (bool(current_key)) {
        Child::process(OutputType{*current_key, std::move(aggregator)});
      }
      Child::end();
    }
  };

  template<ExecutionType ET, typename Child, typename ... X>
  inline decltype(auto) wrap(X&& ... x) {
    using Ctrl = traits::operator_control_t<Child>;
    static_assert(!Ctrl::is_reversed, "GroupByAdjacent does not support reverse iteration. "
      "Consider to use `with_buffer()` for the closest downstream `reverse()` operator.");

    return parent.template wrap<ET, Execution<Child>, Args&, X...>(
      args, std::forward<X>(x)...
    );
  }
};
} // namespace coll
