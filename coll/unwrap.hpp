#pragma once

#include <stdexcept>
#include <type_traits>

#include "base.hpp"

namespace coll {
namespace traits {
namespace details {
template<typename T>
auto is_unwrappable_impl(int) -> decltype(
  *std::declval<T&>(),
  std::true_type{}
);

template<typename T>
std::false_type is_unwrappable_impl(...);
} // namespace details

template<typename T>
using is_unwrappable = decltype(details::is_unwrappable_impl<T>(0));
} // namespace trais

struct UnwrapArgs {};

inline UnwrapArgs unwrap() { return {}; }

template<typename T>
struct UnwrapOrArgs {
  T default_value;
};

template<typename T>
inline UnwrapOrArgs<T> unwrap_or(T&& default_value) {
  return {std::forward<T>(default_value)};
}

// Note(zzxx): Unwrappable means unwrap-pable, instead of un-wrappable.
// E.g., std::optional, iterator, pointer, etc.
template<typename Unwrappable,
  std::enable_if_t<traits::is_unwrappable<Unwrappable>::value>* = nullptr>
inline decltype(auto) operator | (Unwrappable&& wrapped, UnwrapArgs) {
  if constexpr (std::is_constructible_v<bool, decltype(wrapped)>) {
    // is wrapped is an iterator, we do not know if it is valid
    if (!bool(wrapped)) {
      throw std::runtime_error("Attempt to unwrap an invalid (or empty) object.");
    }
  }
  return *wrapped;
}

template<typename Unwrappable, typename T,
  std::enable_if_t<traits::is_unwrappable<Unwrappable>::value>* = nullptr>
inline auto operator | (Unwrappable&& wrapped, const UnwrapOrArgs<T>& arg) {
  if (!bool(wrapped)) {
    return arg.default_value;
  }
  return *wrapped;
}

template<typename Parent,
  typename P = traits::remove_cvr_t<Parent>,
  std::enable_if_t<traits::is_pipe_operator<P>::value>* = nullptr>
inline auto operator | (Parent&& parent, UnwrapArgs) {
  return parent | map([](auto&& wrapped) -> decltype(auto) {
    return wrapped | UnwrapArgs{};
  });
}

template<typename Parent, typename T,
  typename P = traits::remove_cvr_t<Parent>,
  std::enable_if_t<traits::is_pipe_operator<P>::value>* = nullptr>
inline auto operator | (Parent&& parent, const UnwrapOrArgs<T>& arg) {
  return parent | map([arg](auto&& wrapped) -> decltype(auto) {
    return wrapped | arg;
  });
}
} // namespace coll
