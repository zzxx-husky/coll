#pragma once

#include <vector>

#include "base.hpp"
#include "reference.hpp"
#include "utils.hpp"

namespace coll {
template<
  typename BufferBuilder = NullArg,
  bool CacheByRef = false
> struct ReverseArgs {
  constexpr static std::string_view name = "reverse";

  BufferBuilder buffer_builder;

  // used by user
  template<typename AnotherBufferBuilder>
  inline ReverseArgs<AnotherBufferBuilder, CacheByRef>
  with_buffer(AnotherBufferBuilder&& another_builder) {
    return {std::forward<AnotherBufferBuilder>(another_builder)};
  }

  template<template<typename ...> class AnotherBufferTemplate>
  inline auto with_buffer() {
    return with_buffer([](auto type) {
      return AnotherBufferTemplate<typename decltype(type)::type>{};
    });
  }

  inline auto with_buffer() {
    return with_buffer<std::vector>();
  }

  inline ReverseArgs<BufferBuilder, true> cache_by_ref() {
    return {std::forward<BufferBuilder>(buffer_builder)};
  }

  // used by operator
  constexpr static bool reverse_with_buffer = !std::is_same<BufferBuilder, NullArg>::value;
  constexpr static bool is_cache_by_ref = CacheByRef;

  template<typename Input>
  using ElemType = std::conditional_t<CacheByRef,
    Reference<traits::remove_vr_t<Input>>,
    traits::remove_cvr_t<Input>
  >;

  template<typename Input>
  using BufferType = decltype(
    std::declval<ReverseArgs<BufferBuilder, CacheByRef>&>()
      .template get_buffer<Input>()
  );

  template<typename Input, typename Elem = ElemType<Input>>
  inline decltype(auto) get_buffer() {
    if constexpr (std::is_same<BufferBuilder, NullArg>::value) {
      return std::vector<Elem>{};
    }
    if constexpr (traits::is_builder<BufferBuilder, Elem>::value) {
      return buffer_builder(Type<Elem>{});
    } else {
      return buffer_builder;
    }
  }
};

template<typename Parent, typename Args>
struct Reverse {
  using InputType = typename Parent::OutputType;
  using OutputType =
    std::conditional_t<!Args::reverse_with_buffer, InputType,
    std::conditional_t<Args::is_cache_by_ref,      traits::remove_vr_t<InputType>&,
    traits::remove_cvr_t<InputType>&
  >>;
      
  Parent parent;
  Args args;

  template<typename Child>
  struct Forward : public Child {
    Args args;
    typename Args::template ElemType<InputType> elem;

    template<typename ... X>
    Forward(const Args& args, X&& ... x):
      args(args),
      Child(std::forward<X>(x) ...) {
    }

    inline void process(InputType e) {
      elem = std::forward<InputType>(e);
      if constexpr (Args::is_cache_by_ref) {
        Child::process(*elem);
      } else {
        Child::process(elem);
      }
    }
  };

  template<typename Child>
  struct WithBuffer : public Child {
    Args args;
    auto_val(buffer,  args.template get_buffer<InputType>());
    auto_val(control, Child::control.forward());

    template<typename ... X>
    WithBuffer(const Args& args, X&& ... x):
      args(args),
      Child(std::forward<X>(x)...) {
    }

    inline void process(InputType e) {
      container_utils::insert(buffer, std::forward<InputType>(e));
    }

    inline void end() {
      for (auto i = std::rbegin(buffer), e = std::rend(buffer);
           i != e && !Child::control.break_now; ++i) {
        if constexpr (Args::is_cache_by_ref) {
          Child::process(**i);
        } else {
          Child::process(*i);
        }
      }
      Child::end();
    }
  };

  template<typename Child>
  struct WithoutBuffer : public Child {
    Args args;
    auto_val(control, Child::control.reverse());

    template<typename ... X>
    WithoutBuffer(const Args& args, X&& ... x):
      args(args),
      Child(std::forward<X>(x)...) {
    }

    inline void process(InputType e) {
      if (unlikely(Child::control.break_now)) {
        this->control.break_now = true;
      } else {
        Child::process(std::forward<InputType>(e));
      }
    }
  };

  template<typename Child, typename ... X>
  inline decltype(auto) wrap(X&& ... x) {
    using Ctrl = traits::control_t<Child>;
    using ProcessType =
      std::conditional_t<!Args::reverse_with_buffer, WithoutBuffer<Child>,
      std::conditional_t<Ctrl::is_reversed,          Forward<Child>,
      WithBuffer<Child>
    >>;
    return parent.template wrap<ProcessType, Args&, X...>(
      args, std::forward<X>(x)...
    );
  }
};

ReverseArgs<> reverse() { return {}; }

template<typename Parent, typename Args,
  std::enable_if_t<Args::name == "reverse">* = nullptr,
  std::enable_if_t<traits::is_coll_operator<Parent>::value>* = nullptr>
inline Reverse<Parent, Args>
operator | (Parent&& parent, Args&& args) {
  return {std::forward<Parent>(parent), std::forward<Args>(args)};
}
} // namespace coll

