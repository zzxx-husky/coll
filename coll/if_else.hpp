#pragma once

#include "base.hpp"

#include "place_holder.hpp"

namespace coll {
struct IfElseArgsTag {};

template<typename Condition, typename IfBuilder, typename ElseBuilder>
struct IfElseArgs {
  using TagType = IfElseArgsTag;

  Condition condition;
  IfBuilder if_builder;
  ElseBuilder else_builder;
};

template<typename Condition, typename IfBuilder, typename ElseBuilder>
inline auto if_else(
  Condition&& condition, IfBuilder&& if_builder, ElseBuilder&& else_builder) {
  return IfElseArgs<
    traits::remove_cvr_t<Condition>,
    traits::remove_cvr_t<IfBuilder>,
    traits::remove_cvr_t<ElseBuilder>>{
    std::forward<Condition>(condition),
    std::forward<IfBuilder>(if_builder),
    std::forward<ElseBuilder>(else_builder)
  };
}

template<typename Parent, typename Args>
struct IfElse {
  using InputType = typename traits::remove_cvr_t<Parent>::OutputType;

  Parent parent;
  Args args;

  template<typename IfChild, typename ElseChild>
  struct Execution {
    template<typename I, typename E>
    Execution(Args& args, I&& if_child, E&& else_child):
      args(args),
      if_child(std::move(if_child)),
      else_child(std::move(else_child)) {
    }

    Args args;
    IfChild if_child;
    ElseChild else_child;
    auto_val(ctrl, default_control());

    inline auto& control() {
      return ctrl;
    }

    inline void process(InputType e) {
      if (args.condition(e)) {
        if (likely(!if_child.control().break_now)) {
          if_child.feed(e);
        }
      } else {
        if (likely(!else_child.control().break_now)) {
          else_child.feed(e);
        }
      }
      if (unlikely(if_child.control().break_now &&
                   else_child.control().break_now)) {
        ctrl.break_now = true;
      }
    }

    inline void start() {
      else_child.start();
      if_child.start();
    }

    inline void end() {
      if constexpr (traits::execution_has_launch<ElseChild>::value) {
        else_child.launch();
      }
      else_child.end();
      if constexpr (traits::execution_has_launch<IfChild>::value) {
        if_child.launch();
      }
      if_child.end();
    }

    template<typename Exec, typename ... ArgT>
    static decltype(auto) execute(ArgT&& ... args) {
      auto exec = Exec(std::forward<ArgT>(args)...);
      exec.start();
      exec.launch();
      exec.end();
    }
  };

  inline decltype(auto) run() {
    auto if_child = args.if_builder(place_holder<InputType>());
    auto else_child = args.else_builder(place_holder<InputType>());
    return parent.template wrap<ExecutionType::Execute,
      Execution<decltype(if_child), decltype(else_child)>>(
      args, std::move(if_child), std::move(else_child)
    );
  }
};

template<typename Parent, typename Args,
  typename P = traits::remove_cvr_t<Parent>,
  typename A = traits::remove_cvr_t<Args>,
  std::enable_if_t<std::is_same<typename A::TagType, IfElseArgsTag>::value>* = nullptr,
  std::enable_if_t<traits::is_pipe_operator<P>::value>* = nullptr>
inline decltype(auto) operator | (Parent&& parent, Args&& args) {
  return IfElse<P, A>{std::forward<Parent>(parent), std::forward<Args>(args)}.run();
}
} // namespace coll

