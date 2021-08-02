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

  template<typename Child>
  struct A : public Child { using Child::Child; };

  template<typename Child>
  struct B : public Child { using Child::Child; };

  template<typename OutputChild, typename BranchChild>
  struct Execution : public A<OutputChild>, public B<BranchChild> {
    template<typename TupleX, typename TupleY,
      size_t XN = std::tuple_size<traits::remove_cvr_t<TupleX>>::value,
      size_t YN = std::tuple_size<traits::remove_cvr_t<TupleY>>::value>
    Execution(Args& args, TupleX&& x, TupleY&& y): Execution(
      args,
      std::forward<TupleX>(x), std::make_index_sequence<XN>{},
      std::forward<TupleY>(y), std::make_index_sequence<YN>{}) {
    }

    template<typename TupleX, typename TupleY, size_t ... XI, size_t ... YI>
    Execution(Args& args,
      TupleX&& x, std::index_sequence<XI...>,
      TupleY&& y, std::index_sequence<YI...>):
      args(args),
      A<OutputChild>(std::get<XI>(x)...),
      B<BranchChild>(std::get<YI>(y)...) {
    }

    Args args;
    auto_val(control, default_control());

    inline void process(InputType e) {
      if (likely(!B<BranchChild>::control.break_now)) {
        B<BranchChild>::process(e);
      }
      if (likely(!A<OutputChild>::control.break_now)) {
        A<OutputChild>::process(std::forward<InputType>(e));
      }
      if (unlikely(B<BranchChild>::control.break_now &&
                   A<OutputChild>::control.break_now)) {
        control.break_now = true;
      }
    }

    inline void start() {
      B<BranchChild>::start();
      A<OutputChild>::start();
    }

    inline void end() {
      B<BranchChild>::end();
      A<OutputChild>::end();
    }

    inline decltype(auto) result() {
      if constexpr (traits::execution_has_result<A<OutputChild>>::value) {
        return A<OutputChild>::result();
      }
    }

    using A<OutputChild>::execution_type;
    using A<OutputChild>::execute;
  };

  template<typename OutputChild, typename ... X>
  struct Helper {
    using OutputType = InputType;

    Parent& parent;
    Args& args;
    std::tuple<X...> x;

    template<typename BranchChild, typename ... Y>
    inline decltype(auto) wrap(Y&& ... y) {
      return parent.template wrap<Execution<OutputChild, BranchChild>, Args&, std::tuple<X...>&>(
        args, x, std::forward_as_tuple(std::forward<Y>(y)...)
      );
    }
  };

  template<typename OutputChild, typename ... X>
  inline decltype(auto) wrap(X&& ... x) {
    return args.pipeline_builder(Helper<OutputChild, X...>{
      parent, args,
      std::forward_as_tuple(std::forward<X>(x)...)
    });
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
