#pragma once

#include "parallel.hpp"
#include "partition.hpp"

namespace coll {
template<typename PipeBuilder, typename KeyBy = Identity::type>
struct ParallelPartitionArgs {
  static constexpr std::string_view name = "parallel_partition";

  size_t parallelism;
  PipeBuilder pipe_builder;
  KeyBy keyby = Identity::value;
  zaf::ActorGroup* actor_group = nullptr;

  auto& execute_by(zaf::ActorGroup& group) {
    actor_group = &group;
    return *this;
  }

  template<typename AnotherKeyBy>
  inline ParallelPartitionArgs<PipeBuilder, AnotherKeyBy>
  key_by(AnotherKeyBy&& another_keyby) {
    return {
      parallelism,
      std::forward<PipeBuilder>(pipe_builder),
      std::forward<AnotherKeyBy>(another_keyby),
      actor_group
    };
  }
};

template<typename PipeBuilder>
ParallelPartitionArgs<PipeBuilder>
parallel_partition(size_t parallelism, PipeBuilder builder) {
  return {parallelism, builder};
}

template<typename Parent, typename Args,
  typename P = traits::remove_cvr_t<Parent>,
  typename A = traits::remove_cvr_t<Args>,
  std::enable_if_t<A::name == "parallel_partition">* = nullptr,
  std::enable_if_t<traits::is_pipe_operator<P>::value>* = nullptr>
inline auto operator | (Parent&& parent, Args&& args) {
  return parent
    | parallel(args.parallelism, [=](size_t, auto in) {
        return in | partition(args.pipe_builder).by(args.keyby);
      })
      .shuffle_by(shuffle::Partition(args.keyby))
      .execute_by(*args.actor_group);
}
} // namespace coll
