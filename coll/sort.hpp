#pragma once

#include <algorithm>
#include <vector>

#include "base.hpp"
#include "container_utils.hpp"
#include "reference.hpp"
#include "utils.hpp"

namespace coll {
template<typename Parent, typename Args>
struct Sort {
  using InputType = typename Parent::OutputType;
  using OutputType = InputType;

  Parent parent;
  Args args;

  template<typename Child>
  struct Execution : public Child {
    // 1. Args, if any
    Args args;

    template<typename ... X>
    Execution(const Args& args, X&& ... x):
      Child(std::forward<X>(x)...),
      args(args) {
      ctrl = Child::control().forward();
    }

    // 2. States, if any
    auto_val(elems, args.template get_buffer<InputType>());
    decltype(std::declval<Child&>().control().forward()) ctrl;

    inline auto& control() {
      return ctrl;
    }

    // 3. Process each input from parent
    inline void process(InputType e) {
      container_utils::insert(elems, std::forward<InputType>(e));
    }

    // 4. End
    inline void sort() {
      using Ctrl = traits::operator_control_t<Child>;
      auto comparator = args.template get_comparator<InputType&, Ctrl::is_reversed>();
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
           i != e && !Child::control().break_now; ++i) {
        if constexpr (Args::is_cache_by_ref) {
          Child::process(**i);
        } else {
          Child::process(*i);
        }
      }
      Child::end();
    }
  };

  template<ExecutionType ET, typename Child, typename ... X>
  inline decltype(auto) wrap(X&& ... x) {
    return parent.template wrap<ET, Execution<Child>>(args, std::forward<X>(x)...);
  }
};


struct SortArgsTag {};

template<
  typename Comparator = NullArg,
  typename BufferBuilder = NullArg,
  bool CacheByRef = false,
  bool Reverse = false
> struct SortArgs {
  using TagType = SortArgsTag;

  // members
  Comparator comparator{};
  BufferBuilder buffer_builder{}; // to be sorted by `std::sort`.

  // used by user
  inline SortArgs<Comparator, BufferBuilder, true, Reverse>
  cache_by_ref() {
    return {
      std::forward<Comparator>(comparator),
      std::forward<BufferBuilder>(buffer_builder)
    };
  }

  template<typename AnotherBufferBuilder>
  inline SortArgs<Comparator, AnotherBufferBuilder, CacheByRef, Reverse>
  buffer(AnotherBufferBuilder&& another_builder) {
    return {
      std::forward<Comparator>(comparator),
      std::forward<AnotherBufferBuilder>(another_builder)
    };
  }

  template<template <typename ...> class AnotherBufferTemplate>
  inline auto buffer() {
    return buffer([](/* Type<> */ auto type) {
      return AnotherBufferTemplate<typename decltype(type)::type>{};
    });
  }

  inline SortArgs<Comparator, BufferBuilder, CacheByRef, true>
  reverse() {
    return {
      std::forward<Comparator>(comparator),
      std::forward<BufferBuilder>(buffer_builder)
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

inline SortArgs<> sort() { return {}; }

template<typename Comparator>
inline SortArgs<Comparator> sort(Comparator&& comparator) {
  return { std::forward<Comparator>(comparator) };
}

template<typename Parent, typename Args,
  typename P = traits::remove_cvr_t<Parent>,
  typename A = traits::remove_cvr_t<Args>,
  std::enable_if_t<traits::is_pipe_operator<P>::value>* = nullptr,
  std::enable_if_t<std::is_same<typename A::TagType, SortArgsTag>::value>* = nullptr>
inline Sort<P, A> operator | (Parent&& parent, Args&& args) {
  return {std::forward<Parent>(parent), std::forward<Args>(args)};
}
} // namespace coll
