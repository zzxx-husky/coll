#pragma once

#include <vector>

#include "coll/to.hpp"

namespace coll {
inline auto to_vector(unsigned reserve_size = 0) {
  return to(anonyc_cv(
    std::vector<typename decltype(_)::type> vec;
    if (reserve_size != 0) {
      vec.reserve(reserve_size);
    }
    return vec;
  ));
}

inline auto to_uset(unsigned reserve_size = 0) {
  return to(anonyc_cv(
    std::unordered_set<typename decltype(_)::type> set;
    if (reserve_size != 0) {
      set.reserve(reserve_size);
    }
    return set;
  ));
}
} // namespace coll
