#pragma once

#include <unordered_set>

#include "base.hpp"
#include "reference.hpp"

namespace coll {
template<template <typename ...> class Set, typename Inserter, bool CacheByRef>
struct DistinctArgs {
  // [](auto& set, auto&& elem) -> bool
  // to insert an elem into set, return value indicates if the elem is distinct.
  Inserter inserter;

  inline DistinctArgs<Set, Inserter, true> cache_by_ref() {
    return {std::forward<Inserter>(inserter)};
  }

  static constexpr bool is_cache_by_ref =
    CacheByRef;

  template<typename E>
  inline static Set<E> make_set() { return {}; }
};

template<typename Parent, typename Args>
struct Distinct {
  using InputType = typename Parent::OutputType;
  using OutputType = typename Parent::OutputType;

  Parent parent;
  Args args;

  template<typename Ctrl, typename ChildProc>
  inline void foreach(Ctrl& ctrl, ChildProc proc) {
    // No matter it's forward iteration or reverse iteration,
    // the outputs of `distinct()` will be eventually the same.
    // One concern is that the output order is different.
    // Generally speaking, `distinct()`, by name, only requires to remove duplication and does not guarantee
    // to output the first occurrance of duplicate inputs.
    //
    // static_assert(!Ctrl::is_reversed, "Distinct does not support reverse iteration. "
    //   "Consider to use `with_buffer()` for the closest downstream `reverse()` operator.");
    using ElemType = std::conditional_t<Args::is_cache_by_ref,
      Reference<traits::remove_vr_t<InputType>>,
      traits::remove_cvr_t<InputType>
    >;
    auto set = Args::template make_set<ElemType>();
    parent.foreach(ctrl,
      [&](InputType elem) {
        if (args.inserter(set, std::forward<InputType>(elem))) {
          proc(elem);
        }
      });
  }
};

template<template <typename ...> class Set, typename Inserter>
inline DistinctArgs<Set, Inserter, false> distinct(Inserter i) {
  return {std::forward<Inserter>(i)};
}

inline auto distinct() {
  return distinct<std::unordered_set>([](auto& set, auto&& e) -> bool {
    return set.insert(std::forward<decltype(e)>(e)).second;
  });
}

template<template <typename ...> class Set>
inline auto distinct() {
  return distinct<Set>([](auto& set, auto&& e) -> bool {
    return set.insert(std::forward<decltype(e)>(e)).second;
  });
}

template<typename Parent,
  template <typename ...> class Set, typename Inserter, bool CacheByRef,
  std::enable_if_t<traits::is_collection<Parent>::value>* = nullptr>
inline Distinct<Parent, DistinctArgs<Set, Inserter, CacheByRef>>
operator | (const Parent& parent, DistinctArgs<Set, Inserter, CacheByRef> args) {
  return {parent, std::move(args)};
}
} // namespace coll
