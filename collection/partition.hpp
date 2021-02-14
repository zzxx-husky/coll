#pragma once

#include "base.hpp"
#include "place_holder.hpp"
#include "utils.hpp"

namespace coll {
template<
  typename PipelineBuilder,
  typename PartitionMapBuilder,
  typename KeyBy = Identity::type
> struct PartitionArgs {
  constexpr static std::string_view name = "partition";

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

/**
 * There are 3 possibilities for the pipeline construction:
 * 1. The pipeline ends with a sink operator that returns a `result`, e.g., `to` and `aggregate`
 *    + Partition outputs (key, result)
 * 2. The pipeline ends with a sink operator that has no `result`, e.g., print
 *    + Partition outputs the key when the partition of the key `end`s.
 * 3. The pipeline ends with a pipe operator
 *    + Partition outputs every elements in the pipeline to child operator
 *    + The `key` can be attached to the elements by a `map` by the user
 **/
template<typename Parent, typename Args>
struct Partition {
  using InputType = typename Parent::OutputType;

  using KeyType = typename Args::template KeyType<InputType>;
  using PipelineType = typename Args::template PipelineType<InputType>;
  constexpr static bool IsPipeOperator   = traits::is_pipe_operator<PipelineType>::value;
  constexpr static bool IsSinkWithRes    = !IsPipeOperator && traits::has_func_result<PipelineType>::value;
  constexpr static bool IsSinkWithoutRes = !IsPipeOperator && !IsSinkWithRes;

  using OutputType =
    std::conditional_t<IsPipeOperator, typename traits::output_type<PipelineType, IsPipeOperator>::type,
    std::conditional_t<IsSinkWithRes , std::pair<KeyType, typename traits::result_type<PipelineType, IsSinkWithRes>::type>,
               /* IsSinkWithoutRes */  KeyType
  >>;

  Parent parent;
  Args args;

  template<typename Child>
  struct Proc : public Child {
    template<typename ... X>
    Proc(const Args& args, X&& ... x):
      args(args),
      Child(std::forward<X>(x)...) {
    }

    // Used when IsPipeOperator
    template<typename I>
    struct PartitionSink {
      KeyType key;
      Proc<Child>& owner;
      auto_val(control, default_control());

      PartitionSink(const KeyType& key, Proc<Child>& owner):
        key(key),
        owner(owner) {
      }

      inline void process(I e) {
        owner.Child::process(std::forward<I>(e));
      }

      inline void end() {}

      constexpr static ExecutionType execution_type = RunExecution;
    };

    static auto get_partition_map(Args& args) {
      if constexpr (IsPipeOperator) {
        using ExecType = decltype(
          std::declval<Args&>().pipeline_builder(std::declval<const KeyType&>(), place_holder<InputType>())
            .template wrap<PartitionSink<OutputType>>(
              std::declval<KeyType>(), std::declval<Proc<Child>&>())
        );
        return args.template make_partition_map<KeyType, ExecType>();
      } else {
        return args.template make_partition_map<KeyType, PipelineType>();
      }
    }

    Args args;
    auto_val(partition_map, get_partition_map(args));

    inline void process(InputType e) {
      auto&& key = args.keyby(e);
      auto iter = partition_map.find(key);
      if (iter == partition_map.end()) {
        const auto& const_key = key;
        if constexpr (IsPipeOperator) {
          iter = partition_map.emplace(
            key,
            args.pipeline_builder(const_key, place_holder<InputType>())
              .template wrap<PartitionSink<OutputType>>(const_key, *this)
          ).first;
        } else {
          iter = partition_map.emplace(
            key,
            args.pipeline_builder(const_key, place_holder<InputType>())
          ).first;
        }
        // end when created
        if (unlikely(iter->second.control.break_now)) {
          end_partition(key, iter->second);
          return;
        }
      }
      auto& pipe = iter->second;
      if (likely(!pipe.control.break_now)) {
        pipe.process(std::forward<InputType>(e));
        if (unlikely(pipe.control.break_now)) {
          end_partition(key, pipe);
        }
      }
    }

    inline void end() {
      for (auto& i : partition_map) {
        if (!i.second.control.break_now) {
          end_partition(i.first, i.second);
        }
      }
      Child::end();
    }

    // just do not want to think about what K and P are
    template<typename K, typename P>
    inline void end_partition(K&& key, P&& part) {
      part.end();
      if constexpr (IsSinkWithRes) {
        Child::process(OutputType{ key, part.result() });
      } else if constexpr (IsSinkWithoutRes) {
        Child::process(key);
      }
    }
  };

  template<typename Child, typename ... X>
  inline decltype(auto) wrap(X&& ... x) {
    using Ctrl = traits::control_t<Child>;
    static_assert(!Ctrl::is_reversed,
      "Partition operator does not support reversion. Use `with_buffer()` for the nearest `reverse`");
    return parent.template wrap<Proc<Child>, Args&, X...>(
      args, std::forward<X>(x)...
    );
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

template<typename PipelineBuilder>
inline auto partition(PipelineBuilder&& partition_map_builder) {
  return partition<std::unordered_map>(std::forward<PipelineBuilder>(partition_map_builder));
}

template<typename Parent, typename Args,
  std::enable_if_t<Args::name == "partition">* = nullptr,
  std::enable_if_t<traits::is_pipe_operator<Parent>::value>* = nullptr>
inline Partition<Parent, Args>
operator | (Parent&& parent, Args&& args) {
  return {std::forward<Parent>(parent), std::forward<Args>(args)};
}
} // namespace coll
