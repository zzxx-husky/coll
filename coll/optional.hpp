#pragma once

#include "inspect.hpp"
#include "map.hpp"
#include "filter.hpp"
#include "flatmap.hpp"
#include "foreach.hpp"
#include "reference.hpp"

namespace coll {
template<typename O, typename Args,
  typename A = traits::remove_cvr_t<Args>,
  std::enable_if_t<std::is_same<typename A::TagType, MapArgsTag>::value>* = nullptr,
  std::enable_if_t<traits::is_optional<traits::remove_cvr_t<O>>::value>* = nullptr>
inline auto operator | (O&& opt, Args&& args) {
  using T = std::invoke_result_t<decltype(args.mapper), decltype(opt.value())>;
  if (bool(opt)) {
    return std::optional<T>{args.mapper(opt.value())};
  } else {
    return std::nullopt;
  }
}

template<typename O, typename Args,
  typename A = traits::remove_cvr_t<Args>,
  std::enable_if_t<std::is_same<typename A::TagType, FilterArgsTag>::value>* = nullptr,
  std::enable_if_t<traits::is_optional<traits::remove_cvr_t<O>>::value>* = nullptr>
inline decltype(auto) operator | (O&& opt, Args&& args) {
  if (bool(opt) && args.filter(opt.value())) {
    return std::forward<O>(opt);
  } else {
    return std::nullopt;
  }
}

template<typename O, typename Args,
  typename A = traits::remove_cvr_t<Args>,
  std::enable_if_t<std::is_same<typename A::TagType, ForeachArgsTag>::value>* = nullptr,
  std::enable_if_t<traits::is_optional<traits::remove_cvr_t<O>>::value>* = nullptr>
inline void operator | (O&& opt, Args&& args) {
  if (bool(opt)) {
    args.action(opt.value());
  }
}

template<typename O, typename Args,
  typename A = traits::remove_cvr_t<Args>,
  std::enable_if_t<std::is_same<typename A::TagType, InspectArgsTag>::value>* = nullptr,
  std::enable_if_t<traits::is_optional<traits::remove_cvr_t<O>>::value>* = nullptr>
inline decltype(auto) operator | (O&& opt, Args&& args) {
  if (bool(opt)) {
    args.process(opt.value());
  }
  return std::forward<O>(opt);
}

template<typename O, typename Args,
  typename A = traits::remove_cvr_t<Args>,
  std::enable_if_t<std::is_same<typename A::TagType, FlatmapArgsTag>::value>* = nullptr,
  std::enable_if_t<traits::is_optional<traits::remove_cvr_t<O>>::value>* = nullptr>
inline auto operator | (O&& opt, Args&& args) {
  if (bool(opt)) {
    args.process(opt.value());
  }
  return std::forward<O>(opt);
}
} // namespace coll
