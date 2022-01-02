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
  struct Execution : public Child {
    Iter left, right;

    template<typename ...X>
    Execution(const Iter& left, const Iter& right, X&& ... x):
      left(left),
      right(right),
      Child(std::forward<X>(x)...) {
    }

    inline void process() {
      using Ctrl = traits::operator_control_t<Child>;
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
    return Child::template execute<Execution<Child>>(
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
  struct Execution : public Child {
    Iter iterable;

    template<typename ... X>
    Execution(const Iter& iterable, X&& ... x):
      iterable(iterable),
      Child(std::forward<X>(x)...) {
    }

    inline void process() {
      using Ctrl = traits::operator_control_t<Child>;
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
    return Child::template execute<Execution<Child>>(
      iterable, std::forward<X>(x)...
    );
  }
};

template<typename Opt>
struct IterateOptional {
  using OutputType = decltype(*std::declval<Opt&>());

  Opt optional;

  template<typename Child>
  struct Execution : public Child {
    Opt optional;

    template<typename ... X>
    Execution(const Opt& optional, X&& ... x):
      optional(optional),
      Child(std::forward<X>(x)...) {
    }

    inline void process() {
      if (!this->control.break_now && bool(optional)) {
        Child::process(*optional);
      }
    }
  };

  template<typename Child, typename ... X>
  inline decltype(auto) wrap(X&& ... x) {
    return Child::template execute<Execution<Child>>(
      optional, std::forward<X>(x)...
    );
  }
};

template<typename IsEmpty, typename Next>
struct Generator {
  using OutputType = typename traits::invocation<Next>::result_t;

  IsEmpty is_empty;
  Next next;

  template<typename Child>
  struct Execution : public Child {
    IsEmpty is_empty;
    Next next;

    template<typename ... X>
    Execution(const IsEmpty& is_empty, const Next& next, X&& ... x):
      is_empty(is_empty),
      next(next),
      Child(std::forward<X>(x)...) {
    }

    inline void process() {
      using Ctrl = traits::operator_control_t<Child>;
      static_assert(!Ctrl::is_reversed, "Generator does not support reverse iteration. "
        "Consider to use `with_buffer()` for the closest downstream `reverse()` operator.");

      while (!is_empty() && !this->control.break_now) {
        Child::process(next());
      }
    }
  };

  template<typename Child, typename ... X>
  inline decltype(auto) wrap(X&& ... x) {
    return Child::template execute<Execution<Child>>(
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

template<typename ParentExecution>
struct PostIterateResultOfExecution {
  using OutputType = typename traits::iterable<traits::execution_result_t<ParentExecution>>::element_t;

  ParentExecution parent;

  template<typename Child>
  struct Execution : public Child {
    template<typename ... X>
    Execution(ParentExecution parent, X&& ... x):
      parent(parent),
      Child(std::forward<X>(x)...) {
    }

    ParentExecution parent;

    template<typename ... Y>
    inline void process(Y&& ... y) {
      parent.process(std::forward<Y>(y)...);
    }

    inline void end() {
      parent.end();
      auto&& r = parent.result();

      using Ctrl = traits::operator_control_t<Child>;
      if constexpr (Ctrl::is_reversed) {
        for (auto i = r.end(), left = r.begin(); i != left && !this->control.break_now;) {
          Child::process(*(--i));
        }
      } else {
        for (auto i = r.begin(), right = r.end(); i != right && !this->control.break_now; ++i) {
          Child::process(*i);
        }
      }
      Child::end();
    }
  };

  template<typename Child, typename ... X>
  inline auto wrap(X&& ... x) {
    return Execution<Child>(std::move(parent), std::forward<X>(x)...);
  }
};
} // namespace coll
