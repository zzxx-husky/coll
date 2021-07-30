#pragma once

#include "traits.hpp"
#include "utils.hpp"

namespace coll {
namespace container_utils {
template<typename C,
  std::enable_if_t<traits::has_reserve<C>::value>* = nullptr>
inline void reserve(C& container, size_t reserve_size) {
  container.reserve(reserve_size);
}

template<typename C,
  std::enable_if_t<!traits::has_reserve<C>::value>* = nullptr>
inline void reserve(C& container, size_t reserve_size) {}

template<typename E, typename C,
  std::enable_if_t<traits::has_emplace<C, E>::value>* = nullptr>
inline void insert(C& container, E&& elem) {
  container.emplace(std::forward<E>(elem));
}

template<typename E, typename C,
  std::enable_if_t<!traits::has_emplace<C, E>::value>* = nullptr,
  std::enable_if_t<traits::has_insert<C, E>::value>* = nullptr>
inline void insert(C& container, E&& elem) {
  container.insert(std::forward<E>(elem));
}

template<typename E, typename C,
  std::enable_if_t<!traits::has_emplace<C, E>::value>* = nullptr,
  std::enable_if_t<traits::has_push_back<C, E>::value>* = nullptr>
inline void insert(C& container, E&& elem) {
  container.push_back(std::forward<E>(elem));
}

template<typename E, typename C,
  std::enable_if_t<!traits::has_emplace<C, E>::value>* = nullptr,
  // std::enable_if_t<!traits::has_insert<C, E>::value>* = nullptr,
  // std::enable_if_t<!traits::has_push_back<C, E>::value>* = nullptr,
  std::enable_if_t<traits::has_push<C, E>::value>* = nullptr>
inline void insert(C& container, E&& elem) {
  container.push(std::forward<E>(elem));
}

// template<typename C>
// void insert(C& container, ...) {
//   static_assert(false, "Container does not provide any of the following member functions for insertion:"
//     " insert, push_back, push, emplace");
// }
} // namespace container_utils

struct DefaultContainerInserter {
  static constexpr auto value = [](auto& container, auto&& e) {
    container_utils::insert(container, std::forward<decltype(e)>(e));
  };

  using type = decltype(value);
};

template<template<typename ...> class ContainerTemplate>
struct ContainerBuilder {
  template<typename T>
  inline ContainerTemplate<T> operator()(Type<T> type) { return {}; };
};
} // namespace coll
