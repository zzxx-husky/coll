#pragma once

#include <vector>
#include <utility>

#include "aggregate.hpp"
#include "base.hpp"
#include "container_utils.hpp"
#include "groupby_adjacent.hpp"
#include "reference.hpp"

namespace coll {
struct GroupByArgsTag {};

template<typename KeyBy,
  typename ValueBy = Identity::type,
  typename Aggregator = NullArg,
  typename AggregateTo = NullArg,
  bool CacheByRef = false,
  bool Adjacenct = false>
struct GroupByArgs {
  using TagType = GroupByArgsTag;
  using AggregatorType = Aggregator;

  KeyBy keyby;
  ValueBy valby = Identity::value;
  Aggregator aggregator;
  AggregateTo aggregate_to;

  // used by user
  template<typename AnotherAggregator, typename AnotherAggregateTo>
  inline GroupByArgs<KeyBy, ValueBy, AnotherAggregator, AnotherAggregateTo, CacheByRef, Adjacenct>
  aggregate(AnotherAggregator a, AnotherAggregateTo b) {
    return {std::forward<KeyBy>(keyby), std::forward<ValueBy>(valby),
            std::forward<AnotherAggregator>(a), std::forward<AnotherAggregateTo>(b)};
  }

  template<typename AnotherAggregator>
  inline GroupByArgs<KeyBy, ValueBy, AnotherAggregator, DefaultContainerInserter::type, CacheByRef, Adjacenct>
  aggregate(AnotherAggregator a) {
    return {std::forward<KeyBy>(keyby), std::forward<ValueBy>(valby),
            std::forward<AnotherAggregator>(a), DefaultContainerInserter::value};
  }

  inline GroupByArgs<KeyBy, ValueBy, Aggregator, AggregateTo, true, Adjacenct>
  cache_by_ref() {
    return {std::forward<KeyBy>(keyby), std::forward<ValueBy>(valby),
            std::forward<Aggregator>(aggregator), std::forward<AggregateTo>(aggregate_to)};
  }

  inline GroupByArgs<KeyBy, ValueBy, Aggregator, AggregateTo, CacheByRef, true>
  adjacent() {
    return {std::forward<KeyBy>(keyby), std::forward<ValueBy>(valby),
            std::forward<Aggregator>(aggregator), std::forward<AggregateTo>(aggregate_to)};
  }

  template<typename AnotherValueBy>
  inline GroupByArgs<KeyBy, AnotherValueBy, Aggregator, AggregateTo, CacheByRef, Adjacenct>
  valueby(AnotherValueBy another_valueby) {
    return {std::forward<KeyBy>(keyby), std::forward<AnotherValueBy>(another_valueby),
            std::forward<Aggregator>(aggregator), std::forward<AggregateTo>(aggregate_to)};
  }

  inline auto count() {
    return aggregate(size_t(0), [](auto& cnt, auto&&) { ++cnt; });
  }

  inline auto to_vector() {
    return aggregate(ContainerBuilder<std::vector>{}, DefaultContainerInserter::value);
  }

  // used by operator
  constexpr static bool is_adjacent = Adjacenct;

  template<typename Input>
  using KeyType = traits::remove_cvr_t<typename traits::invocation<KeyBy, traits::remove_vr_t<Input>&>::result_t>;

  template<typename Input>
  using ValueType = typename traits::invocation<ValueBy, traits::remove_vr_t<Input>&>::result_t;

  template<typename Input,
    typename Val = ValueType<Input>>
  using ROV = std::conditional_t<CacheByRef,
    Reference<traits::remove_vr_t<Val>>,
    traits::remove_cvr_t<Val>
  >;

  template<typename Input, typename Elem = ROV<Input>>
  inline static constexpr bool is_builder() {
    return traits::is_builder<Aggregator, Elem>::value;
  }

  template<typename Input, typename Elem = ROV<Input>>
  inline decltype(auto) get_aggregator() {
    if constexpr (traits::is_builder<Aggregator, Elem>::value) {
      return aggregator(Type<Elem>{});
    } else {
      return aggregator;
    }
  }
};

template<typename KeyBy>
inline GroupByArgs<KeyBy> groupby(KeyBy keyby) {
  return {std::forward<KeyBy>(keyby)};
}

inline auto groupby() {
  return groupby(Identity::value);
}

template<typename Parent, typename Args,
  typename P = traits::remove_cvr_t<Parent>,
  typename A = traits::remove_cvr_t<Args>,
  std::enable_if_t<std::is_same<typename A::TagType, GroupByArgsTag>::value>* = nullptr,
  std::enable_if_t<!A::is_adjacent>* = nullptr,
  std::enable_if_t<traits::is_pipe_operator<P>::value>* = nullptr>
inline auto operator | (Parent&& parent, Args&& args) {
  using InputType = typename P::OutputType;
  using KeyType = typename A::template KeyType<InputType>;
  if constexpr (std::is_same_v<typename A::AggregatorType, NullArg>) {
    using ValType = decltype(std::declval<A&>().valby(std::declval<InputType>()));
    return parent | aggregate(std::unordered_map<KeyType, ValType>{},
                      [&](auto& map, InputType e) {
                        auto key = args.keyby(e);
                        auto iter = map.find(key);
                        if (iter == map.end()) {
                          map.emplace(std::forward<decltype(key)>(key), args.valby(e));
                        } else {
                          auto val = args.valby(e);
                          iter->second = std::forward<decltype(val)>(val);
                        }
                      });
  } else {
    using ValType = decltype(std::declval<A&>().template get_aggregator<InputType>());
    if constexpr (A::template is_builder<InputType>()) {
      return parent | aggregate(std::unordered_map<KeyType, ValType>(),
                        [&](auto& map, InputType e) {
                          auto&& key = args.keyby(e);
                          auto iter = map.find(key);
                          if (iter == map.end()) {
                            iter = map.emplace(key, args.template get_aggregator<InputType>()).first;
                          }
                          args.aggregate_to(iter->second, args.valby(e));
                        });
    } else {
      return parent | aggregate(std::unordered_map<KeyType, ValType>(),
                        [&](auto& map, InputType e) {
                          args.aggregate_to(map[args.keyby(e)], args.valby(e));
                        });
    }
  }
}

// Override for adjacenct groupby
template<typename Parent, typename Args,
  typename P = traits::remove_cvr_t<Parent>,
  typename A = traits::remove_cvr_t<Args>,
  std::enable_if_t<std::is_same<typename A::TagType, GroupByArgsTag>::value>* = nullptr,
  std::enable_if_t<A::is_adjacent>* = nullptr,
  std::enable_if_t<traits::is_pipe_operator<P>::value>* = nullptr>
inline GroupByAdjacent<P, A>
operator | (Parent&& parent, Args&& args) {
  return {std::forward<Parent>(parent), std::forward<Args>(args)};
}
} // namespace coll
