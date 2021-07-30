#pragma once

#include "place_holder.hpp"
#include "utils.hpp"

namespace coll {
struct PartitionArgsTag {};

template<
  typename PipelineBuilder,
  typename PartitionMapBuilder,
  typename KeyBy = Identity::type
> struct PartitionArgs {
  using TagType = PartitionArgsTag;

  PipelineBuilder pipeline_builder;
  PartitionMapBuilder partition_map_builder;
  KeyBy keyby = Identity::value;

  template<typename AnotherKeyBy>
  inline PartitionArgs<PipelineBuilder, PartitionMapBuilder, AnotherKeyBy>
  by(AnotherKeyBy&& another_keyby) {
    return {
      std::forward<PipelineBuilder>(pipeline_builder),
      std::forward<PartitionMapBuilder>(partition_map_builder),
      std::forward<AnotherKeyBy>(another_keyby)
    };
  }

  template<typename Input>
  using KeyType = traits::remove_cvr_t<
    typename traits::invocation<KeyBy, Input>::result_t
  >;

  template<typename Input>
  using PipelineType = typename traits::invocation<
    PipelineBuilder, const KeyType<Input>&, PlaceHolder<Input>
  >::result_t;

  template<typename K, typename V>
  inline auto make_partition_map() {
    return partition_map_builder(Type<K>{}, Type<V>{});
  }
};

template<template<typename ...> class PartitionMapTemplate, typename PipelineBuilder>
inline auto partition(PipelineBuilder&& pipeline_builder) {
  auto partition_map_builder = [](auto key_type, auto val_type) {
    return PartitionMapTemplate<
      typename decltype(key_type)::type,
      typename decltype(val_type)::type
    >{};
  };
  return PartitionArgs<PipelineBuilder, decltype(partition_map_builder)> {
    std::forward<PipelineBuilder>(pipeline_builder), partition_map_builder
  };
}

template<typename PartitionMapClass, typename PipelineBuilder>
inline auto partition(PipelineBuilder&& pipeline_builder) {
  return PartitionArgs<PipelineBuilder, PartitionMapClass> {
    std::forward<PipelineBuilder>(pipeline_builder), PartitionMapClass{} 
  };
}

template<typename PipelineBuilder>
inline auto partition(PipelineBuilder&& builder) {
  return partition<std::unordered_map>(std::forward<PipelineBuilder>(builder));
}
} // namespace coll
