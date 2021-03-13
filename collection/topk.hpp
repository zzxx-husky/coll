#pragma once

#include <queue>
#include <vector>

namespace coll {
template<typename T, typename Cmp = std::greater<T>>
struct TopK {
  std::priority_queue<T, std::vector<T>, Cmp> elems;
  const size_t K;

  TopK(size_t K): K(K) {}

  TopK(size_t K, Cmp comparator):
    K(K),
    elems(comparator) {
  }

  template<typename U>
  void push(U&& e) {
    elems.push(std::forward<U>(e));
    if (elems.size() > K) {
      elems.pop();
    }
  }

  inline const T& top() const { return elems.top(); }

  inline void pop() { elems.pop(); }

  inline bool empty() const { return elems.empty(); }
};

template<typename T>
inline TopK<T> topk(size_t k) {
  return {k};
}

template<typename T, typename Cmp>
inline TopK<T, Cmp> topk(size_t k, Cmp comparator) {
  return {k, std::forward<Cmp>(comparator)};
}

template<typename Comparator>
struct TopKBuilder {
  const size_t K;
  Comparator cmp;

  template<typename AnotherComparator>
  inline TopKBuilder<AnotherComparator> with(AnotherComparator another_cmp) {
    return {K, std::forward<AnotherComparator>(another_cmp)};
  }

  template<typename By>
  inline auto by(By cmp_by) {
    return with([=](const auto& a, const auto& b) mutable {
      return cmp_by(b) < cmp_by(a);
    });
  }

  // builder function
  template<typename Elem>
  inline auto operator()(Type<Elem>) const {
    if constexpr (std::is_same<Comparator, NullArg>::value) {
      return TopK<Elem>{K};
    } else {
      return TopK<Elem, Comparator>{K, cmp};
    }
  }
};

inline TopKBuilder<NullArg> topk(size_t K) {
  return {K};
}
} // namespace coll
