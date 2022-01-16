#pragma once

#include <functional>
#include <string_view>

#include "control.hpp"
#include "traits.hpp"

namespace coll {

struct NullArg {};

struct ExecutionBase {};

enum ExecutionType {
  /**
   * Construct tells that the construction of object will be handled by child.
   * Child -> Parent
   **/
  Construct = 0,
  /**
   * Object means only to make the execution object.
   * The execution object will be executed.
   * Child -> Parent or Parent -> Child.
   **/
  Object = 1,
  /**
   * Execute means that the constructed execution will be `process`ed and then `end`ed.
   * Result of the execution will be outputed, if any.
   * Child -> Parent or Parent -> Child.
   **/
  Execute = 2
};

/**
 * This is only a template to describe what an Executon is like.
 * Used only for traits.
 *
 * Input: the type of input elements of the execution
 *
 * If this is not a source execution, i.e., it has a parent, the Execution class will be a base class of the parent Execution class.
 * If this is not a sink execution, i.e., it has a child, the Execution class should have a base class which is the child Execution class.
 * If this is a sink execution, it should inherit ExecutionBase.
 **/
template<typename Input>
struct ExecutionLike : public ExecutionBase /* or public ChildExecutionLike */ {
  /**
   * Constructor.
   * The constructor usually takes a list of arguments by a template list, i.e., template<typename ... ArgT>
   * The constructor takes the necessary arguments from the head of the argument list to initialize itself,
   * then pass the rest arguments to the construtor of the child Execution class, e.g.,
   *
   * template<A, B, ... ArgT>
   * ExecutionLike(a, b, ... args):
   *   member_a(a), member_b(b),
   *   Child(std::forward(args) ...) {
   * }
   **/
  ExecutionLike() = default;

  /**
   * The arguments of the operator. Optional. However, it should be necessary for most cases.
   **/
  NullArg args;

  /**
   * The states of the operator. Optional.
   **/
  NullArg states;

  /**
   * An important member named `control` that is an `Control<>` object.
   * A sink execution must declare a `control` member.
   * A non-sink execution may inherit the `control` member from child Execution class, or declare a new and independent one.
   **/
  DefaultControl ctrl = default_control();
  inline DefaultControl& control() { return ctrl; }

  /**
   * Actions to take before the first input element has been `process`ed.
   * We can call the child to `start` by Child::start.
   **/
  inline void start() {}

  /**
   * To process each input element. Required. We can pass elements to child by Child::process.
   **/
  inline void process(Input) {}

  /**
   * Actions to take after the last input element has been `process`ed.
   * We can call the child to `end` by Child::end.
   **/
  inline void end() {}

  /**
   * A sink execution must declare a `result` function which is used to obtain the processing result.
   * The return value can be void if the sink execution has no result to return.
   **/
  inline void result() {}

  /**
   * A sink execution must declare `execute`.
   * A non-sink execution may inherit them from child Execution class, or declare new and independent ones.
   *
   * Exec is the final class that contains the logics of the Execution classes of all the operators in the pipeline
   * args are the arguments to initialize all the Execution classes, ordered from the source execution to the sink execution
   **/
  template<typename Exec, typename ... ArgT>
  static decltype(auto) execute(ArgT&& ... args) {}
};

namespace traits {
namespace details {
template<typename T, typename RT = traits::remove_cvr_t<T>>
auto is_pipe_operator(int) -> decltype(
  // has declared type OutputType
  std::declval<traits::remove_cvr_t<typename RT::OutputType>>(),
  // has a member function `wrap` that accepts an Execution class
  std::declval<RT&>().template wrap<
    ExecutionType::Execute, ExecutionLike<typename RT::OutputType>>(),
  std::declval<RT&>().template wrap<
    ExecutionType::Object, ExecutionLike<typename RT::OutputType>>(),
  std::true_type{}
);

template<typename T>
std::false_type is_pipe_operator(...);

template<typename T>
auto execution_has_result(int) -> decltype(
  // has member function `result`
  std::declval<T&>().result(),
  // the return value of `result()` is not void
  // only void* cannot be incremented
  ++std::declval<traits::remove_cvr_t<decltype(std::declval<T&>().result())>*&>(),
  std::true_type{}
);

template<typename T>
std::false_type execution_has_result(...);

template<typename T>
auto execution_has_start(int) -> decltype(
  // has member function `result`
  std::declval<T&>().start(),
  std::true_type{}
);

template<typename T>
std::false_type execution_has_start(...);

template<typename T>
auto execution_has_end(int) -> decltype(
  // has member function `result`
  std::declval<T&>().end(),
  std::true_type{}
);

template<typename T>
std::false_type execution_has_end(...);

template<typename T, typename ... ArgT>
auto execution_has_run(int) -> decltype(
  // has member function `result`
  std::declval<T&>().run(std::declval<ArgT>() ...),
  std::true_type{}
);

template<typename T, typename ... ArgT>
std::false_type execution_has_run(...);

template<typename T, typename Cond>
auto satisfy(Cond cond) -> decltype(
  cond(std::declval<T&>()),
  std::true_type{}
);

template<typename T>
std::false_type satisfy(...);
} // namespace details

template<typename T>
using is_pipe_operator = decltype(details::is_pipe_operator<T>(0));

template<typename E>
using is_execution = std::is_base_of<ExecutionBase, E>;

template<typename T>
using operator_control_t = typename traits::remove_cvr_t<decltype(std::declval<T&>().control())>;

template<typename T>
using execution_has_result = decltype(details::execution_has_result<T>(0));

template<typename T>
using execution_has_start = decltype(details::execution_has_start<T>(0));

template<typename T>
using execution_has_end = decltype(details::execution_has_end<T>(0));

template<typename T, typename ... ArgT>
using execution_has_run = decltype(details::execution_has_run<T, ArgT ...>(0));

template<typename T, bool enable = true>
struct operator_output {
  using type = typename T::OutputType;
};

template<typename T>
struct operator_output<T, false> {
  using type = void;
};

template<typename T, bool enable = true>
using operator_output_t = typename operator_output<T, enable>::type;

template<typename T, bool enable = true>
struct execution_result {
  using type = decltype(std::declval<T&>().result());
};

template<typename T>
struct execution_result<T, false> {
  using type = void;
};

template<typename T, bool enable = true>
using execution_result_t = typename execution_result<T, enable>::type;
} // namespace traits
} // namespace coll
