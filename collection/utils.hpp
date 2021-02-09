#pragma once

#define likely(x)   __builtin_expect((x),1)
#define unlikely(x) __builtin_expect((x),0)

#define auto_val(x, y) decltype(y)  x = y;
//#define auto_ref(x, y) decltype((y)) x = y;

namespace coll {
// a struct storing type information
template<typename T>
struct Type {
  using type = T;
};

template<typename By>
inline auto compare_by(By by) {
  // mutable in case `by` is mutable
  return [by](const auto& a,const auto& b) mutable {
    return by(a) < by(b);
  };
}

template<typename By>
inline auto rev_compare_by(By by) {
  // mutable in case `by` is mutable
  return [by](const auto& a,const auto& b) mutable {
    return by(b) < by(a);
  };
}

struct Identity {
  static constexpr auto value = [](auto&& v) -> auto&& {
    return std::forward<decltype(v)>(v);
  };

  using type = decltype(value);
};

struct LessThan {
  static constexpr auto value = [](const auto& a, const auto& b) {
    return a < b;
  };

  using type = decltype(value);
};
} // namespace coll
