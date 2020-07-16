#pragma once

#include "base.hpp"

namespace coll {
template<typename I>
struct Range {
  using OutputType = I&;

  I left, right;

  template<typename Ctrl, typename ChildProc>
  inline void foreach(Ctrl& ctrl, ChildProc proc) {
    if constexpr (Ctrl::is_reversed) {
      for (auto i = right; i != left && !ctrl.break_now;) {
        proc(--i);
      }
    } else {
      for (auto i = left; i != right && !ctrl.break_now; ++i) {
        proc(i);
      }
    }
  }
};
} // namespace coll
