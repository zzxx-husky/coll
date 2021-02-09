#pragma once

#include <functional>
#include <string_view>

namespace coll {
template<bool Reverse>
struct Control {
  using ReverseType = Control<!Reverse>;
  using ForwardType = Control<false>;

  // used by child operator to notify parent to break iteration
  bool break_now = false;
  // whether to reversely iterate the elements
  constexpr static bool is_reversed = Reverse;

  // reverse the iteration order based on the current order.
  inline Control<!Reverse> reverse() {
    return {break_now};
  }

  // make the iteration order to be forward
  inline Control<false> forward() {
    return {break_now};
  }
};

inline Control<false> default_control() { return {}; }

using DefaultControl = decltype(default_control());

struct NullArg {};

enum ExecutionType {
  // ConstructExecution of child will be inherited by RunExecution of the parent
  ConstructExecution,
  // RunExecution of child will be overrided by RunExecution of the parent
  // RunExecution of child will be inherited by ConstructExecution of the parent
  RunExecution
};

struct Left {
private:
  Left() = default;
public:
  const static Left value;
};

struct Right {
private:
  Right() = default;
public:
  const static Right value;
};

template<typename ParentInput>
struct CollProcessLike {
  // This is only a template and used only by traits
  CollProcessLike() = default;
  // 1. [Optional] Operator specific arguments
  NullArg args;
  // 2. [Optional] Operator specific states
  NullArg states;
  // 3. [Optional] A new control name `control`, or inherit from Child class
  DefaultControl control = default_control();
  // 4. [Required] A `process` function to process inputs from parent.
  inline void process(ParentInput) {}
  // 5. [Optional] An `end` function that will be called after
  //    all the inputs from parent are processed
  inline void end() {}
  // 6. [Conditional] ExecutionType
  constexpr static ExecutionType execution_type = RunExecution;
  // 7. [Conditional] Execution function
  template<typename Exec, typename ... ArgT>
  static decltype(auto) execution(ArgT&& ... args) {}
};

namespace traits {
namespace details {
template<typename T, typename RT = traits::remove_cvr_t<T>>
auto is_coll_operator(int) -> decltype(
  std::declval<traits::remove_cvr_t<typename RT::OutputType>>(),
  std::declval<RT&>().template wrap<CollProcessLike<typename RT::OutputType>>(),
  std::true_type{}
);

template<typename T>
std::false_type is_coll_operator(...);

template<typename T>
auto has_control(int) -> decltype(
  std::declval<T&>().control,
  std::true_type{}
);

template<typename T>
std::false_type has_control(...);

template<typename T>
auto has_end(int) -> decltype(
  std::declval<T&>().end(),
  std::true_type{}
);

template<typename T>
std::false_type has_end(...);

template<typename T>
auto has_child(int) -> decltype(
  std::declval<T>().child,
  std::true_type{}
);

template<typename T>
std::false_type has_child(...);

template<typename T>
struct parent_operator {
  using type = void;
};

template<template<typename, typename...> class Child, typename Parent, typename ... Args>
struct parent_operator<Child<Parent, Args ...>> {
  using type = Parent;
};

template<typename T, bool exist>
struct control {
  using type = traits::remove_cvr_t<decltype(std::declval<T&>().control)>;
};

template<typename T>
struct control<T, false> {
  using C = decltype(std::declval<T&>().child);
  using type = typename control<C, decltype(has_control<C>(0))::value>::type;
};
} // namespace details

template<typename T>
using is_coll_operator = decltype(details::is_coll_operator<T>(0));

template<typename T>
using parent_operator_t = typename details::parent_operator<T>::type;

template<typename T>
using has_control = decltype(details::has_control<T>(0));

template<typename T>
using has_child = decltype(details::has_child<T>(0));

template<typename T>
using has_end = decltype(details::has_end<T>(0));

template<typename T>
using control_t = typename details::control<T, has_control<T>::value>::type;
} // namespace traits
} // namespace coll
