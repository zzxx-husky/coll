#pragma once

#include <unordered_set>

#include "base.hpp"
#include "reference.hpp"

namespace coll {
template<typename SetBuilder, typename Inserter, bool CacheByRef>
struct DistinctArgs {
  constexpr static std::string_view name = "distinct";

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

  template<typename Input>
  using SetType = decltype(
    std::declval<DistinctArgs<SetBuilder, Inserter, CacheByRef>&>()
      .template make_set<Input>()
  );

  template<typename Input, typename Elem = ElemType<Input>>
  inline decltype(auto) make_set() {
    if constexpr (traits::is_builder<SetBuilder, Elem>::value) {
      return set_builder(Type<Elem>{});
    } else {
      return set_builder;
    }
  }
};

template<typename Parent, typename Args>
struct Distinct {
  using InputType = typename traits::remove_cvr_t<Parent>::OutputType;
  using OutputType = typename traits::remove_cvr_t<Parent>::OutputType;

  Parent parent;
  Args args;

  template<typename Child>
  struct Proc : public Child {
    Args args;
    auto_val(set, args.template make_set<InputType>());

    template<typename ...X>
    Proc(const Args& args, X&& ... x):
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
    return parent.template wrap<Proc<Child>, Args&, X...>(
      args, std::forward<X>(x)...
    );
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

template<typename Parent, typename Args,
  std::enable_if_t<Args::name == "distinct">* = nullptr,
  std::enable_if_t<traits::is_pipe_operator<Parent>::value>* = nullptr>
inline Distinct<Parent, Args>
operator | (Parent&& parent, Args&& args) {
  return {std::forward<Parent>(parent), std::forward<Args>(args)};
}
} // namespace coll
