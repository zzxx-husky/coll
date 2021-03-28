#pragma once

#define likely(x)   __builtin_expect((x),1)
#define unlikely(x) __builtin_expect((x),0)

#define auto_val(x, y) decltype(y) x = y;
#define auto_ref(x, y) std::remove_reference_t<decltype(y)>& x = y;

#define auto_val2(A, x, y) decltype(std::declval<A&>().y) x = y;
#define auto_ref2(A, x, y) std::remove_reference_t<decltype(std::declval<A&>().y)>& x = y;

namespace coll {
// a struct storing type information
template<typename T>
struct Type {
  using type = T;
};

// identity function
struct Identity {
  static constexpr auto value = [](auto&& v) -> decltype(auto) {
    return std::forward<decltype(v)>(v);
  };

  using type = decltype(value);
};
} // namespace coll
