#pragma once

#include <algorithm>
#include <vector>

#include "base.hpp"
#include "container_utils.hpp"
#include "reference.hpp"
#include "utils.hpp"

namespace coll {
template<
  typename BufferBuilder = NullArg,
  typename Comparator = NullArg,
  bool CacheByRef = false,
  bool Reverse = false
> struct SortArgs {
  static constexpr std::string_view name = "sort";

  // members
  BufferBuilder buffer_builder; // to be sorted by `std::sort`.
  Comparator comparator;

  // used by user
  inline SortArgs<BufferBuilder, Comparator, true, Reverse>
  cache_by_ref() {
    return {
      std::forward<BufferBuilder>(buffer_builder),
      std::forward<Comparator>(comparator)
    };
  }

  template<typename AnotherComparator>
  inline SortArgs<BufferBuilder, AnotherComparator, CacheByRef, Reverse>
  by(AnotherComparator&& another_comparator) {
    return {
      std::forward<BufferBuilder>(buffer_builder),
      std::forward<AnotherComparator>(another_comparator)
    };
  }

  template<typename AnotherBufferBuilder>
  inline SortArgs<AnotherBufferBuilder, Comparator, CacheByRef, Reverse>
  buffer(AnotherBufferBuilder&& another_builder) {
    return {std::forward<AnotherBufferBuilder>(another_builder)};
  }

  template<template <typename ...> class AnotherBufferTemplate>
  inline auto buffer() {
    return buffer([](/* Type<> */ auto type) {
      return AnotherBufferTemplate<typename decltype(type)::type>{};
    });
  }

  inline SortArgs<BufferBuilder, Comparator, CacheByRef, true>
  reverse() {
    return {
      std::forward<BufferBuilder>(buffer_builder),
      std::forward<Comparator>(comparator)
    };
  }

  // used by operator
  constexpr static bool is_cache_by_ref = CacheByRef;

  template<typename Input, bool ControlReverse,
    bool IsComparator = traits::is_invocable<Comparator, Input, Input>::value,
    bool IsMapper = traits::is_invocable<Comparator, Input>::value,
    bool IsNullArg = std::is_same<Comparator, NullArg>::value>
  inline auto get_comparator() {
    static_assert(IsComparator || IsMapper || IsNullArg,
      "Invalid user-defined comparator for Sort operator.");

    if constexpr (IsComparator) {
      if constexpr (Reverse != ControlReverse) {
        return [=](const auto& a, const auto& b) { return comparator(b, a); };
      } else {
        return comparator;
      }
    } else if constexpr (IsMapper) {
      if constexpr (Reverse != ControlReverse) {
        return [=](const auto& a, const auto& b) { return comparator(b) < comparator(a); };
      } else {
        return [=](const auto& a, const auto& b) { return comparator(a) < comparator(b); };
      }
    } else /* if constexpr (IsNullArg) */ {
      if constexpr (Reverse != ControlReverse) {
        return [](const auto& a, const auto& b) { return b < a; };
      } else {
        return [](const auto& a, const auto& b) { return a < b; };
      }
    }
  }

  template<typename Input>
  using BufferType = decltype(
    std::declval<SortArgs<BufferBuilder, Comparator, CacheByRef, Reverse>&>()
      .template get_buffer<Input>()
  );

  template<typename Input,
    typename Elem = std::conditional_t<CacheByRef,
      Reference<traits::remove_vr_t<Input>>,
      traits::remove_cvr_t<Input>
  >> inline decltype(auto) get_buffer() {
    if constexpr (std::is_same<BufferBuilder, NullArg>::value) {
      return std::vector<Elem>{};
    } else if constexpr (traits::is_builder<BufferBuilder, Elem>::value) {
      return buffer_builder(Type<Elem>{});
    } else {
      return buffer_builder;
    }
  }
};

template<typename Parent, typename Args>
struct Sort {
  using InputType = typename Parent::OutputType;
  using OutputType = InputType;
  using BufferType = typename Args::template BufferType<InputType>;

  Parent parent;
  Args args;

  template<typename Child>
  struct Proc : public Child {
    // 1. Args, if any
    Args args;

    template<typename ... X>
    Proc(const Args& args, X&& ... x):
      args(args),
      Child(std::forward<X>(x)...) {
    }

    // 2. States, if any
    auto_val(elems,   args.template get_buffer<InputType>());
    auto_val(control, Child::control.forward());

    // 3. Process each input from parent
    inline void process(InputType e) {
      container_utils::insert(elems, std::forward<InputType>(e));
    }

    // 4. End
    inline void sort() {
      using Ctrl = traits::control_t<Child>;
      auto comparator = args.template get_comparator<InputType, Ctrl::is_reversed>();
      if constexpr (Args::is_cache_by_ref) {
        std::sort(elems.begin(), elems.end(), [&](auto& ref_a, auto& ref_b) {
          return comparator(*ref_a, *ref_b);
        });
      } else {
        std::sort(elems.begin(), elems.end(), comparator);
      }
    }

    inline void end() {
      sort();
      for (auto i = elems.begin(), e = elems.end();
           i != e && !Child::control.break_now; ++i) {
        if constexpr (Args::is_cache_by_ref) {
          Child::process(**i);
        } else {
          Child::process(*i);
        }
      }
      Child::end();
    }
  };

  template<typename Child, typename ... X>
  inline decltype(auto) wrap(X&& ... x) {
    return parent.template wrap<Proc<Child>, Args&, X...>(args, std::forward<X>(x)...);
  }
};

inline SortArgs<> sort() { return {}; }

template<typename Operator, typename Args,
  typename Parent = traits::remove_cvr_t<Operator>,
  std::enable_if_t<Args::name == "sort">* = nullptr,
  std::enable_if_t<traits::is_pipe_operator<Parent>::value>* = nullptr>
inline Sort<Parent, Args>
operator | (Operator&& parent, Args&& args) {
  return {std::forward<Operator>(parent), std::forward<Args>(args)};
}
} // namespace coll
