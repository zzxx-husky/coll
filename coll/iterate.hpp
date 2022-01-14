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
        for (auto i = right; i != left && !this->control().break_now;) {
          Child::process(*(--i));
        }
      } else {
        for (auto i = left; i != right && !this->control().break_now; ++i) {
          Child::process(*i);
        }
      }
    }
  };

  template<ExecutionType ET, typename Child, typename ... X>
  inline decltype(auto) wrap(X&& ... x) {
    if constexpr (ET == Construct) {
      return Child::template construct<ExecutionType::Execute, Execution<Child>>(
        left, right, std::forward<X>(x)...
      );
    } else if constexpr (ET == Execute) {
      return Child::template execute<Execution<Child>>(
        left, right, std::forward<X>(x)...
      );
    } else {
      return Execution<Child>(left, right, std::forward<X>(x)...);
    }
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
             i != e && !this->control().break_now; ++i) {
          Child::process(*i);
        }
      } else {
        for (auto i = std::begin(iterable), e = std::end(iterable);
             i != e && !this->control().break_now; ++i) {
          Child::process(*i);
        }
      }
    }
  };

  template<ExecutionType ET, typename Child, typename ... X>
  inline decltype(auto) wrap(X&& ... x) {
    if constexpr (ET == Execute) {
      return Child::template execute<Execution<Child>>(
        iterable, std::forward<X>(x)...
      );
    } else if constexpr (ET == Construct) {
      return Child::template construct<ExecutionType::Execute, Execution<Child>>(
        iterable, std::forward<X>(x)...
      );
    } else {
      return Execution<Child>(iterable, std::forward<X>(x)...);
    }
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
      if (!this->control().break_now && bool(optional)) {
        Child::process(*optional);
      }
    }
  };

  template<ExecutionType ET, typename Child, typename ... X>
  inline decltype(auto) wrap(X&& ... x) {
    if constexpr (ET == Execute) {
      return Child::template execute<Execution<Child>>(
        optional, std::forward<X>(x)...
      );
    } else if constexpr (ET == Construct) {
      return Child::template construct<ExecutionType::Execute, Execution<Child>>(
        optional, std::forward<X>(x)...
      );
    } else {
      return Execution<Child>(optional, std::forward<X>(x)...);
    }
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

      while (!is_empty() && !this->control().break_now) {
        Child::process(next());
      }
    }
  };

  template<ExecutionType ET, typename Child, typename ... X>
  inline decltype(auto) wrap(X&& ... x) {
    if constexpr (ET == Execute) {
      return Child::template execute<Execution<Child>>(
        is_empty, next, std::forward<X>(x)...
      );
    } else if constexpr (ET == Object) {
      return Execution<Child>(is_empty, next, std::forward<X>(x)...);
    } else {
      return Child::template construct<ExecutionType::Execute, Execution<Child>>(
        is_empty, next, std::forward<X>(x)...
      );
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
        for (auto i = r.end(), left = r.begin(); i != left && !this->control().break_now;) {
          Child::process(*(--i));
        }
      } else {
        for (auto i = r.begin(), right = r.end(); i != right && !this->control().break_now; ++i) {
          Child::process(*i);
        }
      }
      Child::end();
    }
  };

  template<ExecutionType ET, typename Child, typename ... X>
  inline auto wrap(X&& ... x) {
    if constexpr (ET == Construct) {
      return Child::template construct<ExecutionType::Object, Execution<Child>>(
        std::move(parent), std::forward<X>(x) ...);
    } else {
      return Execution<Child>(std::move(parent), std::forward<X>(x)...);
    }
  }
};

template<typename Iter,
  std::enable_if_t<traits::is_iterator<Iter>::value>* = nullptr>
inline IterateByIterator<Iter> iterate(Iter start, Iter end) {
  return {std::forward<Iter>(start), std::forward<Iter>(end)};
}

template<typename Next,
  // the Next is callable without input, remove_cvr_t in case the return value is a reference
  traits::remove_cvr_t<decltype(std::declval<Next&>()())>* = nullptr,
  typename DecayedNextReturnType = traits::remove_cvr_t<decltype(std::declval<Next&>()())>,
  // the return value of Next is not void.
  std::enable_if_t<!std::is_same<void, DecayedNextReturnType>::value>* = nullptr>
inline auto generate(Next next) {
  auto always_true = []() { return true; };
  return Generator<decltype(always_true), Next>{always_true, std::forward<Next>(next)};
}

// For persistent iterable, keep the iterators of it
template<typename Coll,
  std::enable_if_t<traits::is_iterable<Coll>::value>* = nullptr,
  typename Iter = typename traits::iterable<Coll>::iterator_t>
inline auto iterate(Coll& c) {
  return iterate(std::begin(c), std::end(c));
}

// For temporal iterable, keep the iterable itself
template<typename Coll,
  std::enable_if_t<traits::is_iterable<Coll>::value>* = nullptr>
inline IterateByIterable<Coll> iterate(Coll&& c) {
  return {std::forward<Coll>(c)};
}

template<typename Opt,
  std::enable_if_t<traits::is_optional<traits::remove_cvr_t<Opt>>::value>* = nullptr>
inline IterateOptional<Opt> iterate(Opt&& o) {
  return {std::forward<Opt>(o)};
}

// For temporal bounded array, keep the pointers of it because idk how to store a temporal array in a class
// E.g., (int []){1, 2, 3}, or "123".
template<typename T,
  typename RT = typename traits::remove_cvr_t<T>,
  std::enable_if_t<traits::is_bounded_array<RT>::value>* = nullptr>
inline auto iterate(T&& arr) { // use T&& such that const char [n] will not be converted to const char*
  return iterate(arr, arr + traits::is_bounded_array<RT>::size);
}

struct PostIterate {};

inline PostIterate iterate() { return {}; }

template<typename Coll,
  std::enable_if_t<traits::is_iterable<Coll>::value>* = nullptr>
inline auto operator | (Coll&& c, PostIterate) {
  return iterate(std::forward<Coll>(c));
}

template<typename Opt,
  std::enable_if_t<traits::is_optional<traits::remove_cvr_t<Opt>>::value>* = nullptr>
inline auto operator | (Opt&& c, PostIterate) {
  return iterate(std::forward<Opt>(c));
}

template<typename Execution,
  typename E = traits::remove_cvr_t<Execution>,
  // it is an execution
  std::enable_if_t<traits::is_execution<E>::value>* = nullptr,
  // the execution has result
  std::enable_if_t<traits::execution_has_result<E>::value>* = nullptr,
  // the result is iterable
  std::enable_if_t<traits::is_iterable<traits::execution_result_t<E>>::value>* = nullptr>
inline auto operator | (Execution&& e, PostIterate) {
  return PostIterateResultOfExecution<E>{std::forward<Execution>(e)};
}

template<typename E, typename ... I>
inline auto elements(I&& ... elems) {
  return iterate(std::array<E, sizeof...(elems)>{std::forward<I>(elems) ...});
}

template<typename ... I>
inline auto elements(I&& ... elems) {
  using E = typename std::common_type_t<I ...>;
  return elements<E, I...>(std::forward<I>(elems) ...);
}
} // namespace coll
