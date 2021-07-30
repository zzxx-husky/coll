#pragma once

#include <unordered_set>

#include "base.hpp"
#include "reference.hpp"

namespace coll {
struct DistinctArgsTag {};

template<typename SetBuilder, typename Inserter, bool CacheByRef>
struct DistinctArgs {
  using TagType = DistinctArgsTag;

  SetBuilder set_builder;
  // [](auto& set, auto& elem) -> bool
  // to insert an elem into set, return value indicates if the elem is distinct.
  Inserter inserter;

  // used by user
  inline DistinctArgs<SetBuilder, Inserter, true> cache_by_ref() {
    return {
      std::forward<SetBuilder>(set_builder),
      std::forward<Inserter>(inserter)
    };
  }

  // used by operator
  template<typename Input>
  using ElemType = std::conditional_t<CacheByRef,
    Reference<traits::remove_vr_t<Input>>,
    traits::remove_cvr_t<Input>
  >;

  template<typename Input, typename Elem = ElemType<Input>>
  inline decltype(auto) make_set() {
    if constexpr (traits::is_builder<SetBuilder, Elem>::value) {
      return set_builder(Type<Elem>{});
    } else {
      return set_builder;
    }
  }
};

template<typename SetBuilder, typename Inserter>
inline DistinctArgs<SetBuilder, Inserter, false>
distinct(SetBuilder&& builder, Inserter&& ins) {
  return {std::forward<SetBuilder>(builder), std::forward<Inserter>(ins)};
}

template<template <typename ...> class Set, typename Inserter>
inline auto distinct(Inserter&& ins) {
  return distinct([](auto type) {
    return Set<typename decltype(type)::type>{};
  }, std::forward<Inserter>(ins));
}

template<template <typename ...> class Set>
inline auto distinct() {
  return distinct([](auto type) {
      return Set<typename decltype(type)::type>{};
    }, [](auto& set, auto&& e) -> bool {
      return set.insert(std::forward<decltype(e)>(e)).second;
    });
}

inline auto distinct() {
  return distinct<std::unordered_set>();
}

template<typename Parent, typename Args>
struct Distinct {
  using InputType = typename traits::remove_cvr_t<Parent>::OutputType;
  using OutputType = typename traits::remove_cvr_t<Parent>::OutputType;

  Parent parent;
  Args args;

  template<typename Child>
  struct Execution : public Child {
    Args args;
    auto_val(set, args.template make_set<InputType>());

    template<typename ...X>
    Execution(const Args& args, X&& ... x):
      args(args),
      Child(std::forward<X>(x)...) {
    }

    inline void process(InputType e) {
      if (args.inserter(set, e)) {
        Child::process(std::forward<InputType>(e));
      }
    }
  };

  // No matter it's forward iteration or reverse iteration,
  // the outputs of `distinct()` will be eventually the same.
  // One concern is that the output order is different.
  // Generally speaking, `distinct()`, by name, only requires to remove duplication and does not guarantee
  // to output the first occurrance of duplicate inputs.
  //
  // static_assert(!Ctrl::is_reversed, "Distinct does not support reverse iteration. "
  //   "Consider to use `with_buffer()` for the closest downstream `reverse()` operator.");
  template<typename Child, typename ... X>
  inline decltype(auto) wrap(X&& ... x) {
    return parent.template wrap<Execution<Child>, Args&, X...>(
      args, std::forward<X>(x)...
    );
  }
};

template<typename Parent, typename Args,
  typename P = traits::remove_cvr_t<Parent>,
  typename A = traits::remove_cvr_t<Args>,
  std::enable_if_t<std::is_same<typename A::TagType, DistinctArgsTag>::value>* = nullptr,
  std::enable_if_t<traits::is_pipe_operator<P>::value>* = nullptr>
inline Distinct<P, A>
operator | (Parent&& parent, Args&& args) {
  return {std::forward<Parent>(parent), std::forward<Args>(args)};
}
} // namespace coll
