#pragma once

#include "base.hpp"
#include "partition_args.hpp"

#include "foreach.hpp"

namespace coll {
/**
 * There are 3 possibilities for the pipeline construction:
 * 1. The pipeline ends with a sink operator that returns a `result`, e.g., `to` and `aggregate`
 *    + Partition outputs (key, result) at the `end`
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
  constexpr static bool IsSinkWithRes    = !IsPipeOperator && traits::execution_has_result<PipelineType>::value;
  constexpr static bool IsSinkWithoutRes = !IsPipeOperator && !IsSinkWithRes;

  using OutputType =
    std::conditional_t<IsPipeOperator, typename traits::operator_output_t<PipelineType, IsPipeOperator>,
    std::conditional_t<IsSinkWithRes , std::pair<KeyType, typename traits::execution_result_t<PipelineType, IsSinkWithRes>>,
               /* IsSinkWithoutRes */  KeyType
  >>;

  Parent parent;
  Args args;

  template<typename Child>
  struct Execution : public Child {
    template<typename ... X>
    Execution(const Args& args, X&& ... x):
      args(args),
      Child(std::forward<X>(x)...) {
    }

    static auto construct_partition_pipeline(Args& args, const KeyType& const_key, Child* child) {
      if constexpr (IsPipeOperator) {
        return args.pipeline_builder(const_key, place_holder<InputType>())
          | foreach([child](auto&& e) {
              child->process(std::forward<decltype(e)>(e));
            });
      } else {
        return args.pipeline_builder(const_key, place_holder<InputType>());
      }
    }

    static auto construct_partition_map(Args& args) {
      if constexpr (IsPipeOperator) {
        using ExecType = decltype(construct_partition_pipeline(std::declval<Args&>(), std::declval<const KeyType&>(), std::declval<Child*>()));
        return args.template make_partition_map<KeyType, ExecType>();
      } else {
        return args.template make_partition_map<KeyType, PipelineType>();
      }
    }

    Args args;
    Child* child = this;
    auto_val(partition_map, construct_partition_map(args));

    inline void process(InputType e) {
      auto&& key = args.keyby(e);
      auto iter = partition_map.find(key);
      if (iter == partition_map.end()) {
        const auto& const_key = key;
        iter = partition_map.emplace(key, construct_partition_pipeline(args, const_key, child)).first;
        // end when created
        if (unlikely(iter->second.control().break_now)) {
          end_partition(key, iter->second);
          return;
        }
      }
      auto& pipe = iter->second;
      // if the partition has ended but there are still new elements coming into this partition
      if (likely(!pipe.control().break_now)) {
        pipe.process(std::forward<InputType>(e));
        if (unlikely(pipe.control().break_now)) {
          end_partition(key, pipe);
        }
      }
    }

    // end when parent ends;
    // does not know how many number of keys there will be,
    // so do not know when *all* the partitions end.
    inline void end() {
      for (auto& i : partition_map) {
        if (!i.second.control().break_now) {
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

  template<ExecutionType ET, typename Child, typename ... X>
  inline decltype(auto) wrap(X&& ... x) {
    using Ctrl = traits::operator_control_t<Child>;
    static_assert(!Ctrl::is_reversed,
      "Partition operator does not support reversion. Use `with_buffer()` for the nearest `reverse`");
    return parent.template wrap<ET, Execution<Child>, Args&, X...>(
      args, std::forward<X>(x)...
    );
  }
};

template<typename Parent, typename Args,
  typename A = traits::remove_cvr_t<Args>,
  typename P = traits::remove_cvr_t<Parent>,
  std::enable_if_t<std::is_same<typename A::TagType, PartitionArgsTag>::value>* = nullptr,
  std::enable_if_t<traits::is_pipe_operator<P>::value>* = nullptr>
inline Partition<P, A>
operator | (Parent&& parent, Args&& args) {
  return { std::forward<Parent>(parent), std::forward<Args>(args) };
}
} // namespace coll
