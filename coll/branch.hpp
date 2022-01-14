#pragma once

#include "base.hpp"

#include "place_holder.hpp"

namespace coll {
struct BranchArgsTag {};

template<typename PipelineBuilder>
struct BranchArgs {
  using TagType = BranchArgsTag;

  PipelineBuilder pipeline_builder;
};

template<typename PipelineBuilder>
inline BranchArgs<PipelineBuilder> branch(PipelineBuilder builder) {
  return {std::forward<PipelineBuilder>(builder)};
}

template<typename Parent, typename Args>
struct Branch {
  using InputType = typename traits::remove_cvr_t<Parent>::OutputType;
  using OutputType = InputType;

  Parent parent;
  Args args;

  template<typename OutputChild, typename BranchChild>
  struct Execution : public OutputChild {
    template<typename B, typename ... X>
    Execution(Args& args, B&& branch_child, X&& ... x):
      args(args),
      branch_child(std::move(branch_child)),
      OutputChild(std::forward<X>(x) ...) {
    }

    Args args;
    BranchChild branch_child;
    auto_val(ctrl, default_control());

    inline auto& control() {
      return ctrl;
    }

    inline void process(InputType e) {
      if (likely(!branch_child.control().break_now)) {
        branch_child.feed(e);
      }
      if (likely(!OutputChild::control().break_now)) {
        OutputChild::process(std::forward<InputType>(e));
      }
      if (unlikely(branch_child.control().break_now &&
                   OutputChild::control().break_now)) {
        ctrl.break_now = true;
      }
    }

    inline void start() {
      branch_child.start();
      OutputChild::start();
    }

    inline void end() {
      if constexpr (traits::execution_has_launch<BranchChild>::value) {
        branch_child.launch();
      }
      branch_child.end();
      OutputChild::end();
    }
  };

  template<ExecutionType ET, typename OutputChild, typename ... X>
  inline decltype(auto) wrap(X&& ... x) {
    auto branch_child = args.pipeline_builder(coll::place_holder<InputType>());
    return parent.template wrap<ET, Execution<OutputChild, decltype(branch_child)>>(
      args, std::move(branch_child), std::forward<X>(x) ...
    );
  }
};

template<typename Parent, typename Args,
  typename P = traits::remove_cvr_t<Parent>,
  typename A = traits::remove_cvr_t<Args>,
  std::enable_if_t<std::is_same<typename A::TagType, BranchArgsTag>::value>* = nullptr,
  std::enable_if_t<traits::is_pipe_operator<P>::value>* = nullptr>
inline Branch<P, A>
operator | (Parent&& parent, Args&& args) {
  return {std::forward<Parent>(parent), std::forward<Args>(args)};
}
} // namespace coll
