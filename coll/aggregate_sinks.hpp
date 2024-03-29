#pragma once

#include "aggregate.hpp"
#include "lambda.hpp"
#include "reference.hpp"
#include "utils.hpp"

namespace coll {
// count
inline auto count() {
  return aggregate(size_t(0), [](auto& cnt, auto&&) { ++cnt; });
}

// max, min
struct MinMaxArgsTag {};

template<typename Comparator, bool Ref>
struct MinMaxArgs {
  using TagType = MinMaxArgsTag;

  Comparator comparator;

  inline MinMaxArgs<Comparator, true> ref() {
    return {std::forward<Comparator>(comparator)};
  }

  constexpr static bool use_ref = Ref;
};

template<typename Comparator>
inline MinMaxArgs<Comparator, false> max(Comparator comp) {
  return {std::forward<Comparator>(comp)};
}

template<typename Comparator>
inline MinMaxArgs<Comparator, false> min(Comparator comp) {
  return {std::forward<Comparator>(comp)};
}

inline auto max() {
  return max([](auto&& a, auto&& b) { return b < a; });
}

inline auto min() {
  return min([](auto&& a, auto&& b) { return a < b; });
}

template<typename Parent, typename Args,
  typename P = traits::remove_cvr_t<Parent>,
  typename A = traits::remove_cvr_t<Args>,
  std::enable_if_t<std::is_same<typename A::TagType, MinMaxArgsTag>::value>* = nullptr,
  std::enable_if_t<traits::is_pipe_operator<P>::value>* = nullptr>
inline auto operator | (Parent&& parent, Args&& args) {
  using InputType = typename P::OutputType;
  using ResultType = std::conditional_t<A::use_ref,
    Reference<typename traits::remove_vr_t<InputType>>,
    std::optional<typename traits::remove_cvr_t<InputType>>
  >;
  auto find_minmax = [&](auto& ref_or_opt, auto&& e) {
    // GCC may warn maybe-uninitialized due to the use of std::optional
    if (!bool(ref_or_opt) || args.comparator(e, *ref_or_opt)) {
      ref_or_opt = e;
    }
  };
  return parent | aggregate(ResultType(), find_minmax);
}

// reduce
struct ReduceArgsTag {};

template<typename R, typename InitVal = NullArg>
struct ReduceArgs {
  using TagType = ReduceArgsTag;

  R reducer;
  InitVal init_val{};

  template<typename AnotherInitVal>
  inline ReduceArgs<R, AnotherInitVal> init(AnotherInitVal i) {
    return {std::forward<R>(reducer), std::forward<AnotherInitVal>(i)};
  }

  constexpr static bool has_init_val =
    !std::is_same<InitVal, NullArg>::value;
};

template<typename R>
inline ReduceArgs<R> reduce(R&& reducer) {
  return {std::forward<R>(reducer)};
}

/**
 * empty coll_operator + no init val => nullopt
 * non empty coll_operator + no init val => some(reduce tail elems on the head elem)
 * empty coll_operator + init val => some(init val)
 * non empty coll_operator + init val => some(reduce elems on the init val)
 **/
template<typename Parent, typename Args,
  typename P = traits::remove_cvr_t<Parent>,
  typename A = traits::remove_cvr_t<Args>,
  std::enable_if_t<std::is_same<typename A::TagType, ReduceArgsTag>::value>* = nullptr,
  std::enable_if_t<traits::is_pipe_operator<P>::value>* = nullptr>
inline auto operator | (Parent&& parent, Args&& args) {
  if constexpr (Args::has_init_val) {
    return std::make_optional(
      parent | aggregate(std::move(args.init_val), args.reducer)
    );
  } else {
    using InputType = typename P::OutputType;
    using ElemType = typename traits::remove_cvr_t<InputType>;
    auto reducer = [&](auto& opt, auto&& e) {
      if (likely(bool(opt))) {
        // GCC may warn maybe-uninitialized due to the use of std::optional
        args.reducer(*opt, std::forward<decltype(e)>(e));
      } else {
        opt = e;
      }
    };
    return parent | aggregate(std::optional<ElemType>(), reducer);
  }
}

inline auto sum() {
  return reduce([](auto& a, auto&& b) {
    a += b;
  });
}

inline auto mul() {
  return reduce([](auto& a, auto&& b) {
    a *= b;
  });
}

// avg
struct AvgArgsTag {};

template<typename Add, typename InitVal = NullArg>
struct AvgArgs {
  using TagType = AvgArgsTag;

  Add add;
  InitVal init_val{};

  template<typename AnotherInitVal>
  inline AvgArgs<Add, AnotherInitVal> init(AnotherInitVal i) {
    return {std::forward<Add>(add), std::forward<AnotherInitVal>(i)};
  }

  // used by operator
  constexpr static bool has_init_val =
    !std::is_same<InitVal, NullArg>::value;
};

template<typename Add>
inline AvgArgs<Add> avg(Add add) {
  return {std::forward<Add>(add)};
}

inline auto avg() {
  return avg([](auto& a, auto&& b) { a += b; });
}

/**
 * empty coll_operator + no init val => nullopt
 * non empty coll_operator + no init val => some(add tail elems to the head elem)
 * empty coll_operator + init val => some(init val)
 * non empty coll_operator + init val => some(add elems to the init val)
 **/
template<typename Parent, typename Args,
  typename P = traits::remove_cvr_t<Parent>,
  typename A = traits::remove_cvr_t<Args>,
  std::enable_if_t<std::is_same<typename A::TagType, AvgArgsTag>::value>* = nullptr,
  std::enable_if_t<traits::is_pipe_operator<P>::value>* = nullptr>
inline auto operator | (Parent&& parent, Args&& args) {
  if constexpr (Args::has_init_val) {
    size_t count = 0;
    auto add = [&](auto&& a, auto&& v) {
      args.add(std::forward<decltype(a)>(a), std::forward<decltype(v)>(v));
      ++count;
    };
    auto agg = parent | aggregate(std::move(args.init_val), add);
    return std::make_optional(agg /= count);
  } else {
    using InputType = typename P::OutputType;
    using ElemType = typename traits::remove_cvr_t<InputType>;
    size_t count = 0;
    auto add = [&](auto& opt, auto&& e) {
      if (likely(count != 0)) {
        args.add(*opt, std::forward<decltype(e)>(e));
      } else {
        opt = e;
      }
      ++count;
    };
    auto sum = parent | aggregate(std::optional<ElemType>(), add);
    return count == 0 ? std::nullopt : std::make_optional(std::move(*sum /= count));
  }
}
} // namespace coll
