#pragma once

#include "base.hpp"
#include "place_holder.hpp"

namespace coll {
template<typename PipelineCtor>
struct BranchArgs {
  constexpr static std::string_view name = "branch";
  // member
  PipelineCtor construct;
};

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
  struct Proc : public A<OutputChild>, public B<BranchChild> {
    template<typename TupleX, typename TupleY,
      size_t XN = std::tuple_size<traits::remove_cvr_t<TupleX>>::value,
      size_t YN = std::tuple_size<traits::remove_cvr_t<TupleY>>::value>
    Proc(Args& args, TupleX&& x, TupleY&& y): Proc(
      args,
      std::forward<TupleX>(x), std::make_index_sequence<XN>{},
      std::forward<TupleY>(y), std::make_index_sequence<YN>{}) {
    }

    template<typename TupleX, typename TupleY, size_t ... XI, size_t ... YI>
    Proc(Args& args,
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

    inline void end() {
      B<BranchChild>::end();
      A<OutputChild>::end();
    }

    using A<OutputChild>::execution_type;
    using A<OutputChild>::execution;
  };

  template<typename OutputChild, typename ... X>
  struct Helper {
    using OutputType = InputType;

    Parent& parent;
    Args& args;
    std::tuple<X...> x;

    template<typename BranchChild, typename ... Y>
    inline decltype(auto) wrap(Y&& ... y) {
      return parent.template wrap<Proc<OutputChild, BranchChild>>(
        args, x, std::forward_as_tuple(std::forward<Y>(y)...)
      );
    }
  };

  template<typename OutputChild, typename ... X>
  inline decltype(auto) wrap(X&& ... x) {
    return args.construct(Helper<OutputChild, X...>{
      parent, args,
      std::forward_as_tuple(std::forward<X>(x)...)
    });
  }
};

template<typename PipelineCtor>
inline BranchArgs<PipelineCtor> branch(PipelineCtor ctor) {
  return {std::forward<PipelineCtor>(ctor)};
}

template<typename Parent, typename Args,
  std::enable_if_t<Args::name == "branch">* = nullptr,
  std::enable_if_t<traits::is_coll_operator<Parent>::value>* = nullptr>
inline Branch<Parent, Args>
operator | (Parent&& parent, Args&& args) {
  return {std::forward<Parent>(parent), std::forward<Args>(args)};
}
} // namespace coll
