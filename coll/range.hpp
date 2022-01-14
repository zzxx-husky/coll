#pragma once

#include "base.hpp"

namespace coll {
template<typename I, typename S = NullArg>
struct Range {
  using OutputType = I&;

  I left, right;
  S step;

  template<typename Child>
  struct Execution : public Child {
    I left, right;
    S step;

    template<typename ...X>
    Execution(I& left, I& right, S& step, X&& ... x):
      left(left),
      right(right),
      step(step),
      Child(std::forward<X>(x)...) {
    }

    inline void launch() {
      using Ctrl = traits::operator_control_t<Child>;
      if constexpr (Ctrl::is_reversed) {
        for (auto i = right; left < i && !this->control().break_now;) {
          if constexpr (std::is_same_v<S, NullArg>) {
            --i;
          } else {
            i -= step;
          }
          Child::process(i);
        }
      } else {
        for (auto i = left; i < right && !this->control().break_now;) {
          Child::process(i);
          if constexpr (std::is_same_v<S, NullArg>) {
            ++i;
          } else {
            i += step;
          }
        }
      }
    }
  };

  template<ExecutionType ET, typename Child, typename ... X>
  inline decltype(auto) wrap(X&& ... x) {
    if constexpr (ET == Construct) {
      return Child::template construct<ExecutionType::Execute, Execution<Child>>(
        left, right, step, std::forward<X>(x)...
      );
    } else if constexpr (ET == Execute) {
      return Child::template execute<Execution<Child>>(
        left, right, step, std::forward<X>(x)...
      );
    } else {
      return Execution<Child>(left, right, step, std::forward<X>(x)...);
    }
  }
};

template<typename E, typename I, typename J, typename S>
inline Range<E, S> range(I&& start, J&& end, S&& step) {
  return {std::forward<I>(start), std::forward<J>(end), std::forward<S>(step)};
}

template<typename E, typename I, typename J>
inline auto range(I&& start, J&& end) {
  return range<E, I, J, NullArg>(
    std::forward<I>(start), std::forward<J>(end), NullArg{});
}

template<typename I, typename J, typename S>
inline auto range(I&& start, J&& end, S&& step) {
  using E = typename traits::remove_cvr_t<std::common_type_t<I, J>>;
  return range<E, I, J, S>(
    std::forward<I>(start), std::forward<J>(end), std::forward<S>(step));
}

template<typename I, typename J>
inline auto range(I&& start, J&& end) {
  using E = typename traits::remove_cvr_t<std::common_type_t<I, J>>;
  return range<E, I, J, NullArg>(
    std::forward<I>(start), std::forward<J>(end), NullArg{});
}

template<typename I>
inline auto range(I&& end) {
  using E = typename traits::remove_cvr_t<I>;
  return range<E, E, I, NullArg>(E{0}, std::forward<I>(end), NullArg{});
}
} // namespace coll
