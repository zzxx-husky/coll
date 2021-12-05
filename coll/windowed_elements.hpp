#pragma once

#include <ostream>
#include <vector>

#include "reference.hpp"
#include "traits.hpp"

namespace coll {

template<typename I, bool CacheByRef>
struct WindowedElements {
public:
  using ElemType = std::conditional_t<CacheByRef,
    Reference<traits::remove_vr_t<I>>,
    traits::remove_cvr_t<I>
  >;

  WindowedElements(size_t size, size_t step):
    window_size(size),
    step(step),
    cur_steps(size) {
    elems.reserve(size);
  }

  WindowedElements(const WindowedElements<I, CacheByRef>&) = default;

private:
  // E may be a value or a reference
  std::vector<ElemType> elems;
  size_t start_idx = 0;
  size_t cur_steps;
  size_t num_elems = 0;
  const size_t step;
  const size_t window_size;

  inline auto& get_by_abs_idx(size_t abs_idx) {
    if constexpr (CacheByRef) {
      return *elems[abs_idx % window_size];
    } else {
      return elems[abs_idx % window_size];
    }
  }

  inline const auto& get_by_abs_idx(size_t abs_idx) const {
    if constexpr (CacheByRef) {
      return *elems[abs_idx % window_size];
    } else {
      return elems[abs_idx % window_size];
    }
  }

public:
  // return `true` when the `cur_steps` is dropped to 0
  // then `cur_steps` will be reset to `step`
  template<typename U>
  bool emplace(U&& in) {
    if (start_idx < window_size) {
      elems.emplace_back(std::forward<U>(in));
      num_elems++;
    } else {
      elems[start_idx % window_size] = std::forward<U>(in);
    }
    start_idx++;
    if (--cur_steps == 0) {
      cur_steps = step;
      return true;
    }
    return false;
  }

  auto& operator[](size_t idx) {
    return get_by_abs_idx(idx + start_idx - num_elems);
  }

  const auto& operator[](size_t idx) const {
    return get_by_abs_idx(idx + start_idx - num_elems);
  }

  template<typename OwnerType>
  struct inner_iterator {
    OwnerType& owner;
    size_t abs_idx;

    inner_iterator<OwnerType>& operator++() {
      ++abs_idx;
      return *this;
    }

    inner_iterator<OwnerType> operator++(int) {
      return {owner, abs_idx++};
    }

    inline friend bool operator==(const inner_iterator<OwnerType>& a, const inner_iterator<OwnerType>& b) {
      return a.abs_idx == b.abs_idx;
    }

    inline friend bool operator!=(const inner_iterator<OwnerType>& a, const inner_iterator<OwnerType>& b) {
      return a.abs_idx != b.abs_idx;
    }

    inline auto& operator*() {
      return owner.get_by_abs_idx(abs_idx);
    }

    inline auto operator->() {
      return &owner.get_by_abs_idx(abs_idx);
    }
  };

  using iterator = inner_iterator<WindowedElements<I, CacheByRef>>;
  using const_iterator = inner_iterator<const WindowedElements<I, CacheByRef>>;

  inline auto begin() { return iterator{*this, start_idx - num_elems}; }
  inline auto begin() const { return const_iterator{*this, start_idx - num_elems}; }

  inline auto end() { return iterator{*this, start_idx}; }
  inline auto end() const { return const_iterator{*this, start_idx}; }

  inline size_t size() const { return num_elems; }

  bool pack_remaining_elements() {
    if (elems.size() < window_size) {
      return !elems.empty();
    }
    if (cur_steps == step) {
      return false;
    }
    num_elems -= std::min(num_elems, cur_steps);
    return num_elems != 0;
  }

  friend std::ostream& operator<<(std::ostream& out, const WindowedElements<I, CacheByRef>& w) {
    auto i = w.begin(), e = w.end();
    if (i != e) {
      out << '[' << *i;
      for (++i; i != e; ++i) {
        out << ", " << *i;
      }
      out << ']';
    } else {
      out << "[]";
    }
    return out;
  }
};

} // namespace coll
