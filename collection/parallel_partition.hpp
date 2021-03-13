#pragma once

#include "queue.hpp"
#include "partition.hpp"

namespace coll {
template<typename Parent, typename Args>
struct ParallelPartition {
  using InputType = typename Parent::OutputType;
  using QueueInputType = traits::remove_cvr_t<InputType>;

  using KeyType = typename Args::template KeyType<InputType>;
  using PipelineType = typename Args::template PipelineType<QueueInputType&>;

  constexpr static bool IsPipeOperator   = traits::is_pipe_operator<PipelineType>::value;
  constexpr static bool IsSinkWithRes    = !IsPipeOperator && traits::execution_has_result<PipelineType>::value;
  constexpr static bool IsSinkWithoutRes = !IsPipeOperator && !IsSinkWithRes;

  using OutputType =
    std::conditional_t<IsPipeOperator, typename traits::operator_output_t<PipelineType, IsPipeOperator>::type,
    std::conditional_t<IsSinkWithRes , std::pair<KeyType, typename traits::remove_cvr_t<typename traits::execution_result_t<PipelineType, IsSinkWithRes>::type>>,
                /* IsSinkWithoutRes */ KeyType
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

    // used for sending inputs to the partitions with each managed by one thread
    std::vector<SWSRQueue<QueueInputType>> swsr_queues = [&]() {
      std::vector<SWSRQueue<QueueInputType>> res(args.num_threads);
      for (int i = 0; i < args.num_threads; i++) {
        res[i].resize(args.queue_length);
      }
      return res;
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
        auto ph = place_holder<QueueInputType&>();
        auto proc = Partition<decltype(ph), Args>{ph, args}
          | foreach([&](OutputType e) {
              mwsr_queue.push(std::forward<OutputType>(e));
            });
        threads.emplace_back([this, pid = i, proc = std::move(proc)]() mutable {
          auto& swsr_queue = swsr_queues.at(pid);
          // no need to worry that proc.control.break_nows becomes true
          // because it does not know how many partitions there will be
          while (swsr_queue.pop([&](QueueInputType& e) {
            proc.process(e);
          }));
          proc.end();
          num_threads_alive.fetch_add(1, std::memory_order_release);
        });
      }
      return threads;
    }();

    inline bool has_threads_alive() {
      return num_threads_alive.load(std::memory_order_acquire) < args.num_threads;
    }

    inline bool try_pop() {
      if (mwsr_queue.not_empty()) {
        mwsr_queue.pop([this](auto& e) {
          this->Child::process(e);
        });
        return true;
      }
      return false;
    }

    inline void process(InputType e) {
      auto&& key = args.keyby(e);
      auto pid = std::hash<KeyType>{}(key) % args.num_threads;
      auto& queue = swsr_queues[pid];
      while (queue.is_full()) {
        if (!try_pop()) {
          if (!has_threads_alive()) {
            this->control.break_now = true;
            return;
          }
          std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
      }
      queue.push(std::forward<InputType>(e));
    }

    inline void end() {
      for (auto& queue : swsr_queues) {
        queue.end();
      }
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
      swsr_queues.clear();
      Child::end();
    }
  };

  template<typename Child, typename ... X>
  inline decltype(auto) wrap(X&& ... x) {
    using Ctrl = traits::operator_control_t<Child>;
    static_assert(!Ctrl::is_reversed,
      "ParallelPartition operator does not support reversion. Use `with_buffer()` for the nearest `reverse`");
    return parent.template wrap<Execution<Child>, Args&, X...>(
      args, std::forward<X>(x)...
    );
  }
};

template<typename Parent, typename Args,
  std::enable_if_t<Args::name == "partition" && Args::is_parallel>* = nullptr,
  std::enable_if_t<traits::is_pipe_operator<Parent>::value>* = nullptr>
inline ParallelPartition<Parent, Args>
operator | (Parent&& parent, Args&& args) {
  return { std::forward<Parent>(parent), std::forward<Args>(args) };
}
} // namespace coll
