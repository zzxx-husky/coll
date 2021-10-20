#pragma once
#if ENABLE_PARALLEL

#include "parallel.hpp"
#include "partition.hpp"

namespace coll {
struct ParallelPartitionArgsTag {};

template<typename PipeBuilder, typename KeyBy = Identity::type>
struct ParallelPartitionArgs {
  using TagType = ParallelPartitionArgsTag;

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
  std::enable_if_t<std::is_same<typename A::TagType, ParallelPartitionArgsTag>::value>* = nullptr,
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
#endif
