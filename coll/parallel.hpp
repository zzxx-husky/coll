#pragma once
#if ENABLE_PARALLEL

#include <vector>

#include "base.hpp"
#include "place_holder.hpp"
#include "shuffle_strategy.hpp"
#include "utils.hpp"

#include "foreach.hpp"

namespace coll {
namespace parallel_utils {
static zaf::ActorSystem actor_system;
} // namespace parallel_utils

struct ParallelArgsTag {};

template<typename PipelineBuilder, typename ShuffleStrat>
struct ParallelArgs {
  using TagType = ParallelArgsTag;

  template<typename I>
  using ShuffleStratType = typename ShuffleStrat::template type<I>;

  // the controls the number of actors that process the data
  size_t parallelism;
  // the builder that builds the logic for processing the data
  PipelineBuilder pipeline_builder;
  ShuffleStrat shuffle_strategy;
  zaf::ActorGroup* actor_group = nullptr;

  template<typename Input>
  using PipelineType = typename traits::invocation<
    PipelineBuilder, size_t, PlaceHolder<Input>
  >::result_t;

  zaf::ActorGroup& get_actor_group() {
    if (!actor_group) {
      return parallel_utils::actor_system;
    }
    return *actor_group;
  }

  auto& execute_by(zaf::ActorGroup& group) {
    actor_group = &group;
    return *this;
  }

  template<typename NewShuffleStrat>
  ParallelArgs<PipelineBuilder, NewShuffleStrat> shuffle_by(NewShuffleStrat&& strat) {
    return {
      parallelism,
      pipeline_builder,
      std::forward<NewShuffleStrat>(strat),
      actor_group
    };
  }
};

template<typename PipelineBuilder>
ParallelArgs<PipelineBuilder, shuffle::OnDemandAssign>
parallel(size_t parallelism, PipelineBuilder builder) {
  return {parallelism, builder, shuffle::OnDemandAssign{}};
}

/**
 * 1. The pipeline ends with a sink operator that returns a `result`
 *    + Parallel outputs the (partition id, result) at the `end`
 * 2. The pipeline ends with a sink operator without a `result`
 *    + Parallel outputs partition id at the `end`
 * 3. The pipeline ends with a pipe operator
 *    + Parallel outputs every elements at the end of the pipeline
 *
 * If the actors `end` by themselves, remaining inputs from the parents will be ignored
 * The effect of `end` from parent to the actors is carried out by in_queue
 * The effect of `end` from actors to child is carried out by out_queue
 *
 * The implementation uses an independent group of actors for execution. The actors are not shared with other parallel operators.
 * TODO(zzxx): support actor sharing
 **/
template<typename Parent, typename Args>
struct Parallel {
  using InputType = typename Parent::OutputType;
  using QueueInputType = traits::remove_cvr_t<InputType>;

  using PipelineType = typename Args::template PipelineType<QueueInputType&>;
  constexpr static bool IsPipeOperator   = traits::is_pipe_operator<PipelineType>::value;
  constexpr static bool IsSinkWithRes    = !IsPipeOperator && traits::execution_has_result<PipelineType>::value;
  constexpr static bool IsSinkWithoutRes = !IsPipeOperator && !IsSinkWithRes;

  using OutputType =
    std::conditional_t<IsPipeOperator, typename traits::operator_output_t<PipelineType, IsPipeOperator>,
    std::conditional_t<IsSinkWithRes , std::pair<size_t, typename traits::remove_cvr_t<typename traits::execution_result_t<PipelineType, IsSinkWithRes>>>,
                /* IsSinkWithoutRes */ size_t
  >>;

  Parent parent;
  Args args;

  template<typename Child>
  struct Execution : public Child {
    template<typename ... X>
    Execution(const Args& args, X&& ... x):
      Child(std::forward<X>(x)...),
      args(args) {
    }

    inline void start() {
      auto& actor_group = this->args.get_actor_group();
      std::vector<zaf::Actor> executors(args.parallelism);
      for (size_t i = 0; i < args.parallelism; i++) {
        executors[i] = actor_group.template spawn<ParallelExecutor>(this->args, i);
      }
      shuffler.initialize(actor_group, executors);
      Child::start();
    }

    Args args;
    unsigned num_termination = 0;
    auto_val(shuffler, args.shuffle_strategy.template create<QueueInputType>());
    // typename Args::template ShuffleStratType<QueueInputType> shuffler = args.create();

    struct ParallelExecutor : public zaf::ActorBehaviorX {
      ParallelExecutor(Args& args, size_t pid):
        args(args), pid(pid) {
      }

      static auto ctor_partition_pipeline(Args& args, size_t pid,
        [[maybe_unused]] zaf::ActorBehaviorX* this_actor,
        [[maybe_unused]] zaf::Actor& res_collector) {
        if constexpr (IsPipeOperator) {
          return args.pipeline_builder(pid, place_holder<QueueInputType&>())
            | foreach([=, &res_collector](OutputType o) {
                this_actor->send(res_collector, codes::Data, std::forward<OutputType>(o));
              });
        } else {
          return args.pipeline_builder(pid, place_holder<QueueInputType&>());
        }
      }

      Args& args;
      size_t pid;
      zaf::Actor res_collector;
      zaf::ActorBehaviorX* this_actor = this;
      auto_val(partition_pipeline, ctor_partition_pipeline(args, pid, this_actor, res_collector));

      void terminate() {
        partition_pipeline.end();
        if constexpr (IsSinkWithRes) {
          this->send(res_collector, codes::Data, std::make_pair(pid, partition_pipeline.result()));
        } else if constexpr (IsSinkWithoutRes) {
          this->send(res_collector, codes::Data, pid);
        }
        this->send(res_collector, codes::Termination);
        this->deactivate();
      }

      zaf::MessageHandlers behavior() override {
        return {
          codes::Downstream - [this](zaf::Actor res_collector) {
            this->res_collector = res_collector;
          },
          codes::Data - [this](QueueInputType& e) {
            partition_pipeline.process(e);
            if (partition_pipeline.control().break_now) {
              terminate();
            }
          },
          codes::Quota - [this](size_t w) {
            this->reply(codes::Quota, w);
          },
          codes::DataWithQuota - [this](QueueInputType& e, size_t w) {
            this->reply(codes::Quota, w);
            partition_pipeline.process(e);
            if (partition_pipeline.control().break_now) {
              terminate();
            }
          },
          codes::Termination - [this]() {
            this->terminate();
          }
        };
      }
    };

    zaf::MessageHandlers receive_handlers{
      codes::Data - [=](OutputType&& o) {
        this->Child::process(std::forward<OutputType>(o));
      },
      codes::Termination - [=]() {
        this->num_termination++;
      }
    };

    inline void receive_results(bool non_blocking) {
      for (bool succ = true; succ && num_termination < args.parallelism;) {
        succ = shuffler.receive(receive_handlers, non_blocking);
      }
    }

    inline void process(InputType e) {
      shuffler.dispatch(std::forward<InputType>(e));
      receive_results(true);
      if (num_termination == args.parallelism) {
        this->control().break_now = true;
      }
    }

    inline void end() {
      shuffler.terminate();
      receive_results(false);
      shuffler.clear();
      Child::end();
    }
  };

  template<ExecutionType ET, typename Child, typename ... X>
  inline decltype(auto) wrap(X&& ... x) {
    using Ctrl = traits::operator_control_t<Child>;
    static_assert(!Ctrl::is_reversed,
      "Parallel operator does not support reversion. Use `with_buffer()` for the nearest `reverse`");
    return parent.template wrap<ET, Execution<Child>, Args&, X...>(
      args, std::forward<X>(x)...
    );
  }
};

template<typename Parent, typename Args,
  typename P = traits::remove_cvr_t<Parent>,
  typename A = traits::remove_cvr_t<Args>,
  std::enable_if_t<std::is_same<typename A::TagType, ParallelArgsTag>::value>* = nullptr,
  std::enable_if_t<traits::is_pipe_operator<P>::value>* = nullptr>
inline Parallel<P, A>
operator | (Parent&& parent, Args&& args) {
  return {std::forward<Parent>(parent), std::forward<Args>(args)};
}
} // namespace coll

#endif
