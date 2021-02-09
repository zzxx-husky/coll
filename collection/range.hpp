#pragma once

#include "base.hpp"

namespace coll {
template<typename I>
struct Range {
  using OutputType = I&;

  I left, right;

  template<typename Child>
  struct Proc : public Child {
    I left, right;

    template<typename ...X>
    Proc(const I& left, const I& right, X&& ... x):
      left(left),
      right(right),
      Child(std::forward<X>(x)...) {
    }

    inline void process() {
      using Ctrl = traits::control_t<Child>;
      if constexpr (Ctrl::is_reversed) {
        for (auto i = right; i != left && !this->control.break_now;) {
          Child::process(--i);
        }
      } else {
        for (auto i = left; i != right && !this->control.break_now; ++i) {
          Child::process(i);
        }
      }
    }
  };

  template<typename Child, typename ... X>
  inline decltype(auto) wrap(X&& ... x) {
    return Child::template execution<Proc<Child>>(
      left, right, std::forward<X>(x)...
    );
  }
};
} // namespace coll
