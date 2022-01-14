#pragma once

#include "base.hpp"

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

  template<typename Child>
  struct A : public Child { using Child::Child; };

  template<typename Child>
  struct B : public Child { using Child::Child; };

  template<typename IfChild, typename ElseChild>
  struct Execution : public A<IfChild>, public B<ElseChild> {
    template<typename TupleX, typename TupleY,
      size_t XN = std::tuple_size<traits::remove_cvr_t<TupleX>>::value,
      size_t YN = std::tuple_size<traits::remove_cvr_t<TupleY>>::value>
    Execution(Args& args, TupleX&& x, TupleY&& y): Execution(
      args,
      std::forward<TupleX>(x), std::make_index_sequence<XN>{},
      std::forward<TupleY>(y), std::make_index_sequence<YN>{}) {
    }

    template<typename TupleX, typename TupleY,
      size_t ... XI, size_t ... YI>
    Execution(Args& args,
      TupleX&& x, std::index_sequence<XI...>,
      TupleY&& y, std::index_sequence<YI...>):
      args(args),
      A<IfChild>(std::get<XI>(x)...),
      B<ElseChild>(std::get<YI>(y)...) {
    }

    Args args;
    auto_val(ctrl, default_control());

    inline auto& control() {
      return ctrl;
    }

    inline void process(InputType e) {
      if (args.condition(e)) {
        if (likely(!A<IfChild>::control().break_now)) {
          A<IfChild>::process(e);
        }
      } else {
        if (likely(!B<ElseChild>::control().break_now)) {
          B<ElseChild>::process(e);
        }
      }
      if (unlikely(
        A<IfChild>::control().break_now &&
        B<ElseChild>::control().break_now)) {
        ctrl.break_now = true;
      }
    }

    inline void start() {
      A<IfChild>::start();
      B<ElseChild>::start();
    }

    inline void end() {
      A<IfChild>::end();
      B<ElseChild>::end();
    }

    template<typename Exec, typename ... ArgT>
    static decltype(auto) execute(ArgT&& ... args) {
      auto exec = Exec(std::forward<ArgT>(args)...);
      exec.start();
      exec.process();
      exec.end();
    }
  };

  template<ExecutionType ETIf, typename IfChild, typename ... X>
  struct WrapElse {
    using OutputType = InputType;

    Parent& parent;
    Args& args;
    std::tuple<X...> x;

    template<ExecutionType ETElse, typename ElseChild, typename ... Y>
    inline decltype(auto) wrap(Y&& ... y) {
      constexpr ExecutionType ET = [&]() {
        if constexpr (ETIf < ETElse) {
          return ETIf;
        } else {
          return ETElse;
        }
      }();
      return parent.template wrap<
        ET, Execution<IfChild, ElseChild>, Args&, std::tuple<X...>>(
        args, std::move(x), std::forward_as_tuple(std::forward<Y>(y)...)
      );
    }
  };

  struct WrapIf {
    using OutputType = InputType;

    Parent& parent;
    Args& args;

    template<ExecutionType ET, typename IfChild, typename ... X>
    inline decltype(auto) wrap(X&& ... x) {
      return args.else_builder(WrapElse<ET, IfChild, X...>{
        parent, args,
        std::forward_as_tuple(std::forward<X>(x)...)
      });
    }
  };

  inline void run() {
    args.if_builder(WrapIf{parent, args});
  }
};

template<typename Parent, typename Args,
  typename P = traits::remove_cvr_t<Parent>,
  typename A = traits::remove_cvr_t<Args>,
  std::enable_if_t<std::is_same<typename A::TagType, IfElseArgsTag>::value>* = nullptr,
  std::enable_if_t<traits::is_pipe_operator<P>::value>* = nullptr>
inline void operator | (Parent&& parent, Args&& args) {
  return IfElse<P, A>{std::forward<Parent>(parent), std::forward<Args>(args)}.run();
}
} // namespace coll

