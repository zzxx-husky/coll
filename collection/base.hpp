#pragma once

#include <functional>

namespace coll {
template<bool Reverse>
struct Control {
  // used by child operator to notify parent to break iteration
  bool break_now = false;
  // whether to reversely iterate the elements
  constexpr static bool is_reversed = Reverse;

  inline Control<!Reverse> reverse() {
    return {break_now};
  }

  inline Control<false> forward() {
    return {break_now};
  }
};

inline Control<false> default_control() { return {}; }

struct NullArg {};

template<typename T>
struct NullTemplateArg {};

namespace traits {
namespace details {
template<typename T>
auto is_collection_impl(int) -> decltype(
  std::declval<T&>().foreach(
    std::declval<Control<true>&>(),
    std::declval<std::function<void(typename T::OutputType)>>()
  ),
  std::declval<T&>().foreach(
    std::declval<Control<false>&>(),
    std::declval<std::function<void(typename T::OutputType)>>()
  ),
  std::true_type{}
);

template<typename T>
std::false_type is_collection_impl(...);

template<typename T>
struct parent_collection_impl {
  using type = void;
};

template<template<typename, typename...> class Child, typename Parent, typename ... Args>
struct parent_collection_impl<Child<Parent, Args ...>> {
  using type = Parent;
};
} // namespace details

template<typename T>
using is_collection = decltype(details::is_collection_impl<T>(0));

template<typename T>
using parent_collection_t = typename details::parent_collection_impl<T>::type;
} // namespace traits
} // namespace coll 
