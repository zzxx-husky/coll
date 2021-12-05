#pragma once
// c
#include <cstring>
// c++
#include <array>
#include <iterator>

#include "base.hpp"
#include "lambda.hpp"
#include "traits.hpp"
// source
#include "iterate.hpp"
#include "range.hpp"
// pipeline
#include "any_all.hpp"
#include "branch.hpp"
#include "concat.hpp"
#include "distinct.hpp"
#include "filter.hpp"
#include "flatmap.hpp"
#include "groupby.hpp"
#include "if_else.hpp"
#include "init_tail.hpp"
#include "inspect.hpp"
#include "link.hpp"
#include "map.hpp"
// #include "parallel.hpp"
// #include "parallel_partition.hpp"
#include "partition.hpp"
#include "reverse.hpp"
#include "sort.hpp"
#include "split.hpp"
#include "take_while.hpp"
#include "unique.hpp"
#include "window.hpp"
#include "zip_with_index.hpp"
// sink
#include "aggregate.hpp"
#include "aggregate_sinks.hpp"
#include "foreach.hpp"
#include "head.hpp"
#include "last.hpp"
#include "print.hpp"
#include "to.hpp"
#include "to_std_containers.hpp"

namespace coll {
template<typename Iter,
  std::enable_if_t<traits::is_iterator<Iter>::value>* = nullptr>
inline IterateByIterator<Iter> iterate(Iter start, Iter end) {
  return {std::forward<Iter>(start), std::forward<Iter>(end)};
}

template<typename Next,
  // the Next is callable without input, remove_cvr_t in case the return value is a reference
  traits::remove_cvr_t<decltype(std::declval<Next&>()())>* = nullptr,
  typename DecayedNextReturnType = traits::remove_cvr_t<decltype(std::declval<Next&>()())>,
  // the return value of Next is not void.
  std::enable_if_t<!std::is_same<void, DecayedNextReturnType>::value>* = nullptr>
inline auto generate(Next next) {
  auto always_true = []() { return true; };
  return Generator<decltype(always_true), Next>{always_true, std::forward<Next>(next)};
}

// For persistent iterable, keep the iterators of it
template<typename Coll,
  std::enable_if_t<traits::is_iterable<Coll>::value>* = nullptr,
  typename Iter = typename traits::iterable<Coll>::iterator_t>
inline auto iterate(Coll& c) {
  return iterate(std::begin(c), std::end(c));
}

// For temporal iterable, keep the iterable itself
template<typename Coll,
  std::enable_if_t<traits::is_iterable<Coll>::value>* = nullptr>
inline IterateByIterable<Coll> iterate(Coll&& c) {
  return {std::forward<Coll>(c)};
}

// For temporal bounded array, keep the pointers of it because idk how to store a temporal array in a class
// E.g., (int []){1, 2, 3}, or "123".
template<typename T,
  typename RT = typename traits::remove_cvr_t<T>,
  std::enable_if_t<traits::is_bounded_array<RT>::value>* = nullptr>
inline auto iterate(T&& arr) { // use T&& such that const char [n] will not be converted to const char*
  return iterate(arr, arr + traits::is_bounded_array<RT>::size);
}

struct PostIterate {};

inline PostIterate iterate() { return {}; }

template<typename Coll,
  std::enable_if_t<traits::is_iterable<Coll>::value>* = nullptr>
inline auto operator | (Coll&& c, PostIterate) {
  return iterate(std::forward<Coll>(c));
}

template<typename Execution,
  typename E = traits::remove_cvr_t<Execution>,
  // it is an execution
  std::enable_if_t<traits::is_execution<E>::value>* = nullptr,
  // the execution has result
  std::enable_if_t<traits::execution_has_result<E>::value>* = nullptr,
  // the result is iterable
  std::enable_if_t<traits::is_iterable<traits::execution_result_t<E>>::value>* = nullptr>
inline auto operator | (Execution&& e, PostIterate) {
  return PostIterateResultOfExecution<E>{std::forward<Execution>(e)};
}

template<typename E, typename ... I>
inline auto elements(I&& ... elems) {
  return iterate(std::array<E, sizeof...(elems)>{std::forward<I>(elems) ...});
}

template<typename ... I>
inline auto elements(I&& ... elems) {
  using E = typename std::common_type_t<I ...>;
  return elements<E, I...>(std::forward<I>(elems) ...);
}

template<typename E, typename I, typename J>
inline Range<E> range(I start, J end) {
  return {start, end};
}

template<typename I, typename J>
inline auto range(I start, J end) {
  using E = typename traits::remove_cvr_t<std::common_type_t<I, J>>;
  return range<E, I, J>(std::forward<I>(start), std::forward<J>(end));
}

template<typename I>
inline auto range(I end) {
  using E = typename traits::remove_cvr_t<I>;
  return range<E, E, I>(E{0}, std::forward<I>(end));
}
} // namespace coll
