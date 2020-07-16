#pragma once

#include "base.hpp"
#include "traits.hpp"

namespace coll {

// Used when the elements are persistent
template<typename Iter>
struct IterateByIterator {
  using OutputType = typename traits::iterator<Iter>::element_t;

  Iter left, right;

  template<typename Ctrl, typename ChildProc>
  inline void foreach(Ctrl& ctrl, ChildProc proc) {
    if constexpr (Ctrl::is_reversed) {
      for (auto i = right; i != left && !ctrl.break_now;) {
        proc(*(--i));
      }
    } else {
      for (auto i = left; i != right && !ctrl.break_now; ++i) {
        proc(*i);
      }
    }
  }
};

// Used when the elements are transient. Keep the elements inside the Iterate.
template<typename Iter>
struct IterateByIterable {
  using OutputType = typename traits::iterable<Iter>::element_t;

  Iter iterable;

  template<typename Ctrl, typename ChildProc>
  inline void foreach(Ctrl& ctrl, ChildProc proc) {
    if constexpr (Ctrl::is_reversed) {
      for (auto i = std::rbegin(iterable), e = std::rend(iterable);
           i != e && !ctrl.break_now; ++i) {
        proc(*i);
      }
    } else {
      for (auto i = std::begin(iterable), e = std::end(iterable);
           i != e && !ctrl.break_now; ++i) {
        proc(*i);
      }
    }
  }
};

template<typename IsEmpty, typename Next>
struct Generator {
  using OutputType = typename traits::invocation<Next>::result_t;

  IsEmpty is_empty;
  Next next;

  template<typename Ctrl, typename ChildProc>
  inline void foreach(Ctrl& ctrl, ChildProc proc) {
    static_assert(!Ctrl::is_reversed, "Generator does not support reverse iteration. "
      "Consider to use `with_buffer()` for the closest downstream `reverse()` operator.");

    while (!is_empty() && !ctrl.break_now) {
      proc(next());
    }
  }

  template<typename AnotherIsEmpty,
    // the IsEmpty is callable without input, and the return value can be converted to bool
    decltype(bool(std::declval<AnotherIsEmpty&>()()))* = nullptr>
  inline Generator<AnotherIsEmpty, Next> until(AnotherIsEmpty another_is_empty) {
    return {std::forward<AnotherIsEmpty>(another_is_empty), std::forward<Next>(next)};
  }

  inline auto times(size_t repeat_times) {
    return until([repeat_times]() mutable {
      return repeat_times-- == 0;
    });
  }
};
} // namespace coll
