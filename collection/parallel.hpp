#pragma once

#include <iostream>
#include <thread>
#include <vector>

#include "base.hpp"
#include "place_holder.hpp"
#include "queue.hpp"
#include "utils.hpp"

#include "foreach.hpp"

namespace coll {
template<typename PipelineBuilder>
struct ParallelArgs {
  constexpr static std::string_view name = "parallel";

  size_t num_threads;
  PipelineBuilder pipeline_builder;

  template<typename Input>
  using PipelineType = typename traits::invocation<
    PipelineBuilder, size_t, PlaceHolder<Input>
  >::result_t;
};

template<typename PipelineBuilder>
ParallelArgs<PipelineBuilder> parallel(size_t num_threads, PipelineBuilder builder) {
  return {num_threads, builder};
}

/**
 * 1. The pipeline ends with a sink operator that returns a `result`
 *    + Parallel outputs the (partition id, result) at the `end`
 * 2. The pipeline ends with a sink operator without a `result`
 *    + Parallel outputs partition id at the `end`
 * 3. The pipeline ends with a pipe operator
 *    + Parallel outputs every elements at the end of the pipeline
 *
 * If the threads `end` by themselves, remaining inputs from the parents will be ignored
 * The effect of `end` from parent to the threads is carried out by swmr_queue
 * The effect of `end` from threads to child is carried out by mwsr_queue
 *
 * The implementation uses an independent group of threads for execution. The threads are not shared with other parallel operators.
 * TODO(zzxx): support thread sharing
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
    std::conditional_t<IsPipeOperator, typename traits::operator_output_t<PipelineType, IsPipeOperator>::type,
    std::conditional_t<IsSinkWithRes , std::pair<size_t, typename traits::remove_cvr_t<typename traits::execution_result_t<PipelineType, IsSinkWithRes>::type>>,
                /* IsSinkWithoutRes */ size_t
  >>;

  using QueueOutputType = traits::remove_cvr_t<OutputType>;

  Parent parent;
  Args args;

  template<typename Child>
  struct Execution : public Child {
    template<typename ... X>
    Execution(const Args& args, X&& ... x):
      args(args),
      Child(std::forward<X>(x)...) {
    }

    Args args;

    // used for sending inputs to threads
    SWMRQueue<QueueInputType> swmr_queue = [&]() {
      SWMRQueue<QueueInputType> queue;
      queue.resize(args.num_threads << 1);
      return queue;
    }();

    // used for collecting outputs from threads
    MWSRQueue<QueueOutputType> mwsr_queue = [&]() {
      MWSRQueue<QueueOutputType> queue;
      queue.resize(args.num_threads << 1);
      return queue;
    }();

    std::atomic<unsigned> num_threads_alive{0};

    std::vector<std::thread> threads = [&]() {
      std::vector<std::thread> threads;
      threads.reserve(args.num_threads);
      for (int i = 0; i < args.num_threads; i++) {
        auto&& e = [&]() -> decltype(auto) {
          if constexpr (IsPipeOperator) {
            return args.pipeline_builder(i, place_holder<QueueInputType&>())
              | foreach([&](OutputType e) {
                  mwsr_queue.push(std::forward<OutputType>(e));
                });
          } else {
            return args.pipeline_builder(i, place_holder<QueueInputType&>());
          }
        }();
        threads.emplace_back(
          [this, pid = i, exec = std::forward<decltype(e)>(e)]() mutable {
            for (bool pop_succ = true;
                 pop_succ && !exec.control.break_now;) {
              // either obtain an elem from the queue
              // or block and wait for a new elem
              // or unblock because the queue is `end`ed
              pop_succ = swmr_queue.pop([&](QueueInputType& e) {
                exec.process(e);
              });
            }
            exec.end();
            if constexpr (IsSinkWithRes) {
              mwsr_queue.push(std::make_pair(pid, exec.result()));
            } else if constexpr (IsSinkWithoutRes) {
              mwsr_queue.push(pid);
            }
            num_threads_alive.fetch_add(1, std::memory_order_release);
          });
      }
      return threads;
    }();

    inline bool try_pop() {
      if (mwsr_queue.not_empty()) {
        mwsr_queue.pop([this](auto& e) {
          this->Child::process(e);
        });
        return true;
      }
      return false;
    }

    inline bool has_threads_alive() {
      return num_threads_alive.load(std::memory_order_acquire) < args.num_threads;
    }

    inline void process(InputType e) {
      while (swmr_queue.is_full()) {
        if (!try_pop()) {
          if (!has_threads_alive()) {
            this->control.break_now = true;
            return;
          }
          std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
      }
      swmr_queue.push(std::forward<InputType>(e));
      while(try_pop());
    }

    // ended by parent, no more inputs
    inline void end() {
      // by swmr_queue to tell the threads to end
      swmr_queue.end();
      while (true) {
        while (try_pop());
        if (has_threads_alive()) {
          std::this_thread::sleep_for(std::chrono::milliseconds(1));
        } else {
          break;
        }
      }
      for (auto& t : threads) {
        t.join();
      }
      threads.clear();
      Child::end();
    }
  };

  template<typename Child, typename ... X>
  inline decltype(auto) wrap(X&& ... x) {
    using Ctrl = traits::operator_control_t<Child>;
    static_assert(!Ctrl::is_reversed,
      "Parallel operator does not support reversion. Use `with_buffer()` for the nearest `reverse`");
    return parent.template wrap<Execution<Child>, Args&, X...>(
      args, std::forward<X>(x)...
    );
  }
};

template<typename Parent, typename Args,
  std::enable_if_t<Args::name == "parallel">* = nullptr,
  std::enable_if_t<traits::is_pipe_operator<Parent>::value>* = nullptr>
inline Parallel<Parent, Args>
operator | (Parent&& parent, Args&& args) {
  return {std::forward<Parent>(parent), std::forward<Args>(args)};
}
} // namespace coll
