#pragma once

#include "base.hpp"
#include "filter.hpp"
#include "map.hpp"
#include "head.hpp"

namespace coll {
struct AnyArgsTag {};

template<typename Condition>
struct AnyArgs {
  using TagType = AnyArgsTag;

  Condition condition;
};

template<typename C>
inline auto any(C&& cond) {
  return AnyArgs<traits::remove_cvr_t<C>>{std::forward<C>(cond)};
}

template<typename Parent, typename Args,
  typename P = traits::remove_cvr_t<Parent>,
  typename A = traits::remove_cvr_t<Args>,
  std::enable_if_t<std::is_same<typename A::TagType, AnyArgsTag>::value>* = nullptr,
  std::enable_if_t<traits::is_pipe_operator<P>::value>* = nullptr>
inline bool operator | (Parent&& parent, Args&& args) {
  auto r = parent
    | filter(std::forward<decltype(args.condition)>(args.condition))
    | map(anony_ac(true))
    | head();
  return bool(r);
}

struct AllArgsTag {};

template<typename Condition>
struct AllArgs {
  using TagType = AllArgsTag;

  Condition condition;
};

template<typename C>
inline auto all(C&& cond) {
  return AllArgs<traits::remove_cvr_t<C>>{std::forward<C>(cond)};
}

template<typename Parent, typename Args,
  typename P = traits::remove_cvr_t<Parent>,
  typename A = traits::remove_cvr_t<Args>,
  std::enable_if_t<std::is_same<typename A::TagType, AllArgsTag>::value>* = nullptr,
  std::enable_if_t<traits::is_pipe_operator<P>::value>* = nullptr>
inline bool operator | (Parent&& parent, Args&& args) {
  auto r = parent
    | filter([
        c = std::forward<decltype(args.condition)>(args.condition)
      ](auto&& v) {
        // find if any element does not satisfy the condition
        return !c(std::forward<decltype(v)>(v));
      })
    | map(anony_ac(true))
    | head();
  return !bool(r);
}
} // namespace coll
