#pragma once

#include "base.hpp"
#include "traits.hpp"

namespace coll {

// Used when the elements are persistent
template<typename Iter>
struct IterateByIterator {
  using OutputType = typename traits::iterator<Iter>::element_t;

  Iter left, right;

  template<typename Child>
  struct IterateByIteratorProc : public Child {
    Iter left, right;

    template<typename ...X>
    IterateByIteratorProc(const Iter& left, const Iter& right, X&& ... x):
      left(left),
      right(right),
      Child(std::forward<X>(x)...) {
    }

    inline void process() {
      using Ctrl = traits::control_t<Child>;
      if constexpr (Ctrl::is_reversed) {
        for (auto i = right; i != left && !this->control.break_now;) {
          Child::process(*(--i));
        }
      } else {
        for (auto i = left; i != right && !this->control.break_now; ++i) {
          Child::process(*i);
        }
      }
    }
  };

  template<typename Child, typename ... X>
  inline decltype(auto) wrap(X&& ... x) {
    return Child::template execution<IterateByIteratorProc<Child>>(
      left, right, std::forward<X>(x)...
    );
  }
};

// Used when the elements are transient. Keep the elements inside the Iterate.
template<typename Iter>
struct IterateByIterable {
  using OutputType = typename traits::iterable<Iter>::element_t;

  Iter iterable;

  template<typename Child>
  struct IterateByIterableProc : public Child {
    Iter iterable;

    template<typename ... X>
    IterateByIterableProc(const Iter& iterable, X&& ... x):
      iterable(iterable),
      Child(std::forward<X>(x)...) {
    }

    inline void process() {
      using Ctrl = traits::control_t<Child>;
      if constexpr (Ctrl::is_reversed) {
        for (auto i = std::rbegin(iterable), e = std::rend(iterable);
             i != e && !this->control.break_now; ++i) {
          Child::process(*i);
        }
      } else {
        for (auto i = std::begin(iterable), e = std::end(iterable);
             i != e && !this->control.break_now; ++i) {
          Child::process(*i);
        }
      }
    }
  };

  template<typename Child, typename ... X>
  inline decltype(auto) wrap(X&& ... x) {
    return Child::template execution<IterateByIterableProc<Child>>(
      iterable, std::forward<X>(x)...
    );
  }
};

template<typename IsEmpty, typename Next>
struct Generator {
  using OutputType = typename traits::invocation<Next>::result_t;

  IsEmpty is_empty;
  Next next;

  template<typename Child>
  struct GeneratorProc : public Child {
    IsEmpty is_empty;
    Next next;

    template<typename ... X>
    GeneratorProc(const IsEmpty& is_empty, const Next& next, X&& ... x):
      is_empty(is_empty),
      next(next),
      Child(std::forward<X>(x)...) {
    }

    inline void process() {
      using Ctrl = traits::control_t<Child>;
      static_assert(!Ctrl::is_reversed, "Generator does not support reverse iteration. "
        "Consider to use `with_buffer()` for the closest downstream `reverse()` operator.");

      while (!is_empty() && !this->control.break_now) {
        Child::process(next());
      }
    }
  };

  template<typename Child, typename ... X>
  inline decltype(auto) wrap(X&& ... x) {
    return Child::template execution<GeneratorProc<Child>>(
      is_empty, next, std::forward<X>(x)...
    );
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
