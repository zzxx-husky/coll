#pragma once
#include <iostream>

#include <algorithm>
#include <vector>

#include "base.hpp"
#include "container_utils.hpp"
#include "reference.hpp"
#include "to.hpp"
#include "utils.hpp"

namespace coll {
template<bool CacheByRef, bool Reverse, typename CmpOrMap = typename LessThan::type>
struct SortArgs {
  CmpOrMap cmp_or_map = LessThan::value;

  static constexpr bool is_cache_by_ref = CacheByRef;

  inline SortArgs<true, Reverse, CmpOrMap> cache_by_ref() {
    return {std::forward<CmpOrMap>(cmp_or_map)};
  }

  template<typename AnotherCmpOrMap>
  inline SortArgs<CacheByRef, Reverse, AnotherCmpOrMap> by(AnotherCmpOrMap another_cmp_or_map) {
    return {std::forward<AnotherCmpOrMap>(another_cmp_or_map)};
  }

  inline SortArgs<CacheByRef, true, CmpOrMap> reverse() {
    return {std::forward<CmpOrMap>(cmp_or_map)};
  }

  template<typename Input,
    std::enable_if_t<traits::is_invocable<CmpOrMap, Input, Input>::value>* = nullptr>
  inline auto get_comparator() {
    if constexpr (Reverse) {
      return [cmp_or_map = std::forward<CmpOrMap>(cmp_or_map)](const auto& a, const auto& b) {
        return cmp_or_map(b, a);
      };
    } else {
      return cmp_or_map;
    }
  }

  template<typename Input,
    std::enable_if_t<traits::is_invocable<CmpOrMap, Input>::value>* = nullptr>
  inline auto get_comparator() {
    if constexpr (Reverse) {
      return [=](const auto& a, const auto& b) {
        return cmp_or_map(b) < cmp_or_map(a);
      };
    } else {
      return [=](const auto& a, const auto& b) {
        return cmp_or_map(a) < cmp_or_map(b);
      };
    }
  }
};

template<typename Parent, typename Args>
struct Sort {
  using InputType = typename Parent::OutputType;
  using OutputType = InputType;
  using ElemType = std::conditional_t<Args::is_cache_by_ref,
    Reference<traits::remove_vr_t<InputType>>,
    traits::remove_cvr_t<InputType>
  >;
  using BufferType = std::vector<ElemType>;

  Parent parent;
  Args args;

  template<typename Ctrl>
  inline BufferType buffer_and_sort(Ctrl& ctrl) {
    BufferType elems;
    parent.foreach(ctrl, [&](InputType elem) {
      container_utils::insert(elems, std::forward<InputType>(elem));
    });
    auto comparator = args.template get_comparator<InputType>();
    if constexpr (Args::is_cache_by_ref) {
      if constexpr (Ctrl::is_reversed) {
        std::sort(elems.begin(), elems.end(), [&](auto& ref_a, auto& ref_b) {
          return comparator(*ref_b, *ref_a);
        });
      } else {
        std::sort(elems.begin(), elems.end(), [&](auto& ref_a, auto& ref_b) {
          return comparator(*ref_a, *ref_b);
        });
      }
    } else {
      if constexpr (Ctrl::is_reversed) {
        std::sort(elems.begin(), elems.end(), [&](auto& a, auto& b) {
          return comparator(b, a);
        });
      } else {
        std::sort(elems.begin(), elems.end(), comparator);
      }
    }
    return elems;
  }

  template<typename Ctrl, typename ChildProc>
  inline void foreach(Ctrl& ctrl, ChildProc proc) {
    auto new_ctrl = ctrl.forward();
    auto elems = buffer_and_sort(new_ctrl);
    for (auto i = elems.begin(), e = elems.end();
         i != e && !ctrl.break_now; ++i) {
      if constexpr (Args::is_cache_by_ref) {
        proc(**i);
      } else {
        proc(*i);
      }
    }
  }

  template<typename ToArgsT, 
    typename Container = typename ToArgsT::template ContainerType<OutputType>,
    std::enable_if_t<std::is_same<BufferType, Container>::value>>
  inline BufferType operator | (ToArgsT to) {
    auto ctrl = default_control();
    auto container = to.template get_container<OutputType>();
    container = std::move(buffer_and_sort(ctrl));
    return container;
  }
};

inline SortArgs<false, false> sort() { return {}; }

template<typename Parent, bool CacheByRef, bool Reverse, typename CmpOrMap,
  std::enable_if_t<traits::is_collection<Parent>::value>* = nullptr>
inline Sort<Parent, SortArgs<CacheByRef, Reverse, CmpOrMap>>
operator | (const Parent& parent, SortArgs<CacheByRef, Reverse, CmpOrMap> args) {
  return {parent, std::move(args)};
}
} // namespace coll
