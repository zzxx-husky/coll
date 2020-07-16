#pragma once

#include <unordered_map>

#include "aggregate.hpp"
#include "container_utils.hpp"
#include "lambda.hpp"
#include "reference.hpp"
#include "topk.hpp"
#include "utils.hpp"

namespace coll {
// count
inline auto count() {
  return aggregate(size_t(0), [](auto& cnt, auto&&) { ++cnt; });
}

// max, min
template<typename Comparator, bool Ref>
struct MinMaxArgs {
  Comparator comparator;

  inline MinMaxArgs<Comparator, true> ref() {
    return {std::forward<Comparator>(comparator)};
  }
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

template<typename Parent, typename Comparator, bool Ref,
  std::enable_if_t<traits::is_collection<Parent>::value>* = nullptr>
inline auto operator | (const Parent& parent, MinMaxArgs<Comparator, Ref> args) {
  using InputType = typename Parent::OutputType;
  using ResultType = std::conditional_t<Ref,
    Reference<typename traits::remove_vr_t<InputType>>,
    std::optional<typename traits::remove_cvr_t<InputType>>
  >;
  auto find_minmax = [&](auto& ref_or_opt, auto&& e) {
    if (!bool(ref_or_opt) || args.comparator(e, *ref_or_opt)) {
      ref_or_opt = e;
    }
  };
  return parent | aggregate(ResultType(), find_minmax);
}

// sum
template<typename Add, typename InitVal = NullArg>
struct SumArgs {
  Add add;
  InitVal init_val;

  constexpr static bool has_init_val =
    !std::is_same<InitVal, NullArg>::value;

  template<typename AnotherInitVal>
  inline SumArgs<Add, AnotherInitVal> init(AnotherInitVal i) {
    return {std::forward<Add>(add), std::forward<AnotherInitVal>(i)};
  }
};

template<typename Add>
inline SumArgs<Add> sum(Add add) {
  return {std::forward<Add>(add)};
}

inline auto sum() {
  return sum([](auto& a, auto&& b) { a += b; });
}

/**
 * empty collection + no init val => nullopt
 * non empty collection + no init val => some(add tail elems to the head elem)
 * empty collection + init val => some(init val)
 * non empty collection + init val => some(add elems to the init val)
 **/
template<typename Parent, typename Add, typename InitVal,
  std::enable_if_t<traits::is_collection<Parent>::value>* = nullptr>
inline auto operator | (const Parent& parent, SumArgs<Add, InitVal> args) {
  if constexpr (args.has_init_val) {
    return std::make_optional(
      parent | aggregate(std::forward<InitVal>(args.init_val), args.add)
    );
  } else {
    using InputType = typename Parent::OutputType;
    using ElemType = typename traits::remove_cvr_t<InputType>;
    auto add = [&](auto& opt, auto&& e) {
      if (likely(bool(opt))) {
        args.add(*opt, std::forward<decltype(e)>(e));
      } else {
        opt = e;
      }
    };
    return parent | aggregate(std::optional<ElemType>(), add);
  }
}

// avg
template<typename Add, typename InitVal = NullArg>
struct AvgArgs {
  Add add;
  InitVal init_val;

  constexpr static bool has_init_val =
    !std::is_same<InitVal, NullArg>::value;

  template<typename AnotherInitVal>
  inline AvgArgs<Add, AnotherInitVal> init(AnotherInitVal i) {
    return {std::forward<Add>(add), std::forward<AnotherInitVal>(i)};
  }
};

template<typename Add>
inline AvgArgs<Add> avg(Add add) {
  return {std::forward<Add>(add)};
}

inline auto avg() {
  return avg([](auto& a, auto& b) { a += b; });
}

/**
 * empty collection + no init val => nullopt
 * non empty collection + no init val => some(add tail elems to the head elem)
 * empty collection + init val => some(init val)
 * non empty collection + init val => some(add elems to the init val)
 **/
template<typename Parent, typename Add, typename InitVal,
  std::enable_if_t<traits::is_collection<Parent>::value>* = nullptr>
inline auto operator | (const Parent& parent, AvgArgs<Add, InitVal> args) {
  if constexpr (args.has_init_val) {
    size_t count = 0;
    auto add = [&](auto&& a, auto&& v) {
      args.add(std::forward<decltype(a)>(a), std::forward<decltype(v)>(v));
      ++count;
    };
    auto agg = parent | aggregate(std::forward<InitVal>(args.init_val), add);
    return std::make_optional(agg /= count);
  } else {
    using InputType = typename Parent::OutputType;
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
