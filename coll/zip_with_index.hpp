#pragma once

#include "map.hpp"

namespace coll {
// `second` is reference if the input `e` is reference
inline auto zip_with_index() {
  return map([index = size_t(0)] (auto&& e) mutable -> std::pair<size_t, decltype(e)> {
    return { index++, std::forward<decltype(e)>(e) };
  });
}
} // namespace coll
