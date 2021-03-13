#pragma once

#include <iterator>
#include <utility>

#include "utils.hpp"

namespace coll {
namespace traits {
namespace details {
template<typename Func, typename ... Args>
auto is_invocable_impl(int) -> decltype (
  std::declval<Func&>()(std::declval<Args&&>() ...),
  std::true_type{}
);

template<typename Func, typename ... Args>
std::false_type is_invocable_impl(...);

template<typename T>
auto is_iterator_impl(int) -> decltype(
  ++std::declval<T&>(),
  *std::declval<T&>(),
  std::true_type{}
);

template<typename T>
std::false_type is_iterator_impl(...);

template<typename T>
auto is_iterable_impl(int) -> decltype(
  ++std::declval<decltype(std::begin(std::declval<T&>()))&>(),
  *std::declval<decltype(std::begin(std::declval<T&>()))&>(),
  std::true_type{}
);

template<typename T>
std::false_type is_iterable_impl(...);

template<typename C>
auto has_reserve_impl(int) -> decltype(
  std::declval<C&>().reserve(1),
  std::true_type{}
);

template<typename C>
std::false_type has_reserve_impl(...);

template<typename C>
auto has_clear_impl(int) -> decltype(
  std::declval<C&>().clear(),
  std::true_type{}
);

template<typename C>
std::false_type has_clear_impl(...);

template<typename C, typename E>
auto has_insert_impl(int) -> decltype(
  std::declval<C&>().insert(std::declval<E&>()),
  std::true_type{}
);

template<typename C, typename E>
std::false_type has_insert_impl(...);

template<typename C, typename E>
auto has_push_back_impl(int) -> decltype(
  std::declval<C&>().push_back(std::declval<E&>()),
  std::true_type{}
);

template<typename C, typename E>
std::false_type has_push_back_impl(...);

template<typename C, typename E>
auto has_push_impl(int) -> decltype(
  std::declval<C&>().push(std::declval<E&>()),
  std::true_type{}
);

template<typename C, typename E>
std::false_type has_push_impl(...);

template<typename C, typename E>
auto has_emplace_impl(int) -> decltype(
  std::declval<C&>().emplace(std::declval<E&>()),
  std::true_type{}
);

template<typename C, typename E>
std::false_type has_emplace_impl(...);

template<typename T, typename E>
auto is_builder_impl(int) -> decltype(
  std::declval<T&>()(Type<E>{}),
  std::true_type{}
);

template<typename T, typename E>
std::false_type is_builder_impl(...);
} // namespace details

template<typename T>
using is_iterator = decltype(details::is_iterator_impl<T>(0));

template<typename T>
using is_iterable = decltype(details::is_iterable_impl<T>(0));

template<typename Func, typename ... Args>
struct invocation {
  using result_t = decltype(std::declval<Func&>()(std::declval<Args&&>() ...));
};

template<typename Func, typename ... Args>
using is_invocable = decltype(details::is_invocable_impl<Func, Args ...>(0));

template<typename T>
struct remove_cvr {
  using type = std::remove_cv_t<std::remove_reference_t<T>>;
};

template<typename T>
struct remove_vr {
  using type = std::remove_volatile_t<std::remove_reference_t<T>>;
};

template<typename T>
using remove_cvr_t = typename remove_cvr<T>::type;

template<typename T>
using remove_vr_t = typename remove_vr<T>::type;

template<typename C, bool enable = true>
struct iterable {
  using iterator_t = decltype(std::begin(std::declval<C&>()));
  using element_t = decltype(*std::begin(std::declval<C&>()));
};

template<typename C>
struct iterable<C, false> {
  using iterator_t = void;
  using element_t = void;
};

template<typename I>
struct iterator {
  using element_t = decltype(*std::declval<I&>());
};

// https://stackoverflow.com/questions/24855160/how-to-tell-if-a-c-template-type-is-c-style-string
template<class T>
struct is_c_str : std::integral_constant<
  bool,
  std::is_same<char const *, typename std::decay_t<T>>::value ||
  std::is_same<char *, typename std::decay_t<T>>::value
> {};

template<typename C>
using has_reserve = decltype(details::has_reserve_impl<C>(0));

template<typename C>
using has_clear = decltype(details::has_clear_impl<C>(0));

template<typename C, typename E>
using has_insert = decltype(details::has_insert_impl<C, E>(0));

template<typename C, typename E>
using has_push_back = decltype(details::has_push_back_impl<C, E>(0));

template<typename C, typename E>
using has_push = decltype(details::has_push_impl<C, E>(0));

template<typename C, typename E>
using has_emplace = decltype(details::has_emplace_impl<C, E>(0));

template<class T>
struct is_bounded_array: std::false_type {};

template<class T, size_t N>
struct is_bounded_array<T[N]> : std::true_type {
  constexpr static size_t size = N;
};

template<typename T, typename E>
using is_builder = decltype(details::is_builder_impl<T, E>(0));

template<typename T, typename E, bool enable = true>
struct builder {
  using result_t = decltype(std::declval<T&>()(Type<E>{}));
};

template<typename T, typename E>
struct builder<T, E, false> {
  using result_t = void;
};
} // namespace traits
} // namespace coll
