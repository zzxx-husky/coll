#pragma once

#include "inspect.hpp"
#include "iterate.hpp"
#include "filter.hpp"
#include "flatmap.hpp"
#include "foreach.hpp"
#include "map.hpp"
#include "reference.hpp"
#include "traits.hpp"
#include "traversal.hpp"

namespace coll {
template<typename Optional, typename Args,
  typename O = traits::remove_cvr_t<Optional>,
  typename A = traits::remove_cvr_t<Args>,
  std::enable_if_t<std::is_same<typename A::TagType, MapArgsTag>::value>* = nullptr,
  std::enable_if_t<traits::is_optional<O>::value>* = nullptr>
inline auto operator | (Optional&& optional, Args&& args) {
  return bool(optional)
    ? std::optional(args.mapper(*optional))
    : std::nullopt;
}

template<typename Optional, typename Args,
  typename O = traits::remove_cvr_t<Optional>,
  typename A = traits::remove_cvr_t<Args>,
  std::enable_if_t<std::is_same<typename A::TagType, FlatmapArgsTag>::value>* = nullptr,
  std::enable_if_t<traits::is_optional<O>::value>* = nullptr>
inline auto operator | (Optional&& optional, Args&& args) {
  using C = decltype(args.mapper(*optional));
  if constexpr (traits::is_iterable<C>::value) {
    using E = std::remove_reference_t<typename traits::iterable<C>::element_t>;
    return bool(optional)
      ? coll::iterate(args.mapper(*optional)) | coll::to_traversal()
      : coll::elements<E>() | coll::to_traversal();
  } else if constexpr (traits::is_pipe_operator<traits::remove_cvr_t<C>>::value) {
    using E = typename traits::remove_cvr_t<C>::OutputType;
    return bool(optional)
      ? args.mapper(*optional) | coll::to_traversal()
      : coll::elements<E>() | coll::to_traversal();
  }
}

template<typename Optional, typename Args,
  typename O = traits::remove_cvr_t<Optional>,
  typename A = traits::remove_cvr_t<Args>,
  std::enable_if_t<std::is_same<typename A::TagType, FilterArgsTag>::value>* = nullptr,
  std::enable_if_t<traits::is_optional<O>::value>* = nullptr>
inline auto operator | (Optional&& optional, Args&& args) {
  return (bool(optional) && args.filter(*optional))
    ? optional
    : std::nullopt;
}

template<typename Optional, typename Args,
  typename O = traits::remove_cvr_t<Optional>,
  typename A = traits::remove_cvr_t<Args>,
  std::enable_if_t<std::is_same<typename A::TagType, ForeachArgsTag>::value>* = nullptr,
  std::enable_if_t<traits::is_optional<O>::value>* = nullptr>
inline void operator | (Optional&& optional, Args&& args) {
  if (bool(optional)) {
    args.action(*optional);
  }
}

template<typename Optional, typename Args,
  typename O = traits::remove_cvr_t<Optional>,
  typename A = traits::remove_cvr_t<Args>,
  std::enable_if_t<std::is_same<typename A::TagType, InspectArgsTag>::value>* = nullptr,
  std::enable_if_t<traits::is_optional<O>::value>* = nullptr>
inline decltype(auto) operator | (Optional&& optional, Args&& args) {
  if (bool(optional)) {
    args.process(*optional);
  }
  return std::forward<Optional>(optional);
}
} // namespace coll
