#pragma once

#include "base.hpp"

namespace coll {
template<typename Parent>
struct ConcatArgs {
  constexpr static std::string_view name = "concat";

  using ParentType = Parent;

  Parent parent;
};

/**
 * Another implementation of Concat is by inheritance:
 *
 *         +-- Parent1 <-- RemoveEnd <-+
 *         |                           |
 * Proc <--+                           +-- Child
 *         |                           |
 *         +-- Parent2 <-- RemoveEnd <-+
 *
 * But we need to upgrade Contrl a bit to support this, i.e.,
 * we register a callback in the Control, the callback will be invoked by the source operator
 * and the callback tells the source operator how to deal with its Process struct and the input arguments.
 * By default, the callback is to construct a Process instance with the args, invoke the instance, and return the `result()` of the instance.
 * By default, if the source is PlaceHolder, the callback is to construct a Process instance and return the instance.
 * Different operators may change the behavior of the callback, but the source has the right to determine.
 * With this callback, Concat is able to know the Process structs and the args of the two source operator,
 * then Concat is able to build a Proc struct just like the above.
 *
 * Virtual inheritance is not used here because it requires an empty constructor for all the Process structures.
 */
template<typename Parent, typename Args>
struct Concat {
  using OutputType = typename std::common_type_t<
    typename traits::remove_cvr_t<Parent>::OutputType,
    typename Args::ParentType::OutputType
  >;

  Parent parent;
  Args args;

  // X includes args to Parent1 and Y, XN = len(args to Parent1)
  // Y includes args to Parent2 and Z, YN = len(args to Parent2)
  // Z includes parent1& and args to Child, ZN = len(args to Child)
  template<typename Parent1, typename Parent2, typename Child,
           size_t XN, size_t YN, size_t ZN>
  struct Proc : public Parent1, public Parent2, public Child {
    template<typename TupleXYZ>
    Proc(TupleXYZ&& xyz): Proc(
      std::forward<TupleXYZ>(xyz),
      std::make_index_sequence<XN>{},
      std::make_index_sequence<YN>{},
      std::make_index_sequence<ZN>{}) {
    }

    template<typename TupleXYZ, size_t ... XI, size_t ... YI, size_t ... ZI>
    Proc(TupleXYZ&& xyz, std::index_sequence<XI...>, std::index_sequence<YI...>, std::index_sequence<ZI...>):
      Parent1(std::get<XI>(xyz)..., static_cast<Child&>(*this)),
      Parent2(std::get<YI>(std::get<XN>(xyz))..., static_cast<Child&>(*this)),
      Child  (std::get<ZI+1>(std::get<YN>(std::get<XN>(xyz)))...) {
    }

    template<typename ... ArgT>
    inline void process(ArgT&& ... args) {
      Parent1::process(std::forward<ArgT>(args) ...);
      Parent2::process(std::forward<ArgT>(args) ...);
    }

    // Invoke the first parent only
    template<typename ... ArgT>
    inline void process(Left, ArgT&& ... args) {
      Parent1::process(std::forward<ArgT>(args) ...);
    }

    // Invoke the second parent only
    template<typename ... ArgT>
    inline void process(Right, ArgT&& ... args) {
      Parent2::process(std::forward<ArgT>(args) ...);
    }

    inline auto end() {
      Parent1::end();
      Parent2::end();
      Child::end();
    }
  };

  template<typename Parent2, typename Child, size_t YN, size_t ZN>
  struct RemoveEnd1 {
    template<typename _>
    RemoveEnd1(_&&) {}

    RemoveEnd1(Child& child):
      child(&child),
      control_ptr(&child.control) {
    }

    Child* child = nullptr;
    typename traits::control_t<Child>* control_ptr = nullptr;
    typename traits::control_t<Child>& control = *control_ptr;

    inline void process(OutputType e) {
      child->process(std::forward<OutputType>(e));
    }

    inline void end() {}

    constexpr static ExecutionType execution_type = ConstructExecution;
    template<typename Parent1, typename ... XYZ, size_t XN = sizeof...(XYZ) - 1>
    static auto execution(XYZ&& ... xyz) {
      return Child::template execution<Proc<Parent1, Parent2, Child, XN, YN, ZN>>(
        std::forward_as_tuple(std::forward<XYZ>(xyz)...)
      );
    };
  };

  template<typename Child, size_t ZN>
  struct RemoveEnd2 {
    template<typename _>
    RemoveEnd2(_&&) {}

    RemoveEnd2(Child& child):
      child(&child),
      control_ptr(&child.control) {
    }

    Child* child = nullptr;
    typename traits::control_t<Child>* control_ptr = nullptr;
    typename traits::control_t<Child>& control = *control_ptr;

    inline void process(OutputType e) {
      child->process(std::forward<OutputType>(e));
    }

    inline void end() {}

    constexpr static ExecutionType execution_type = ConstructExecution;
    template<typename Parent2, typename ... YZ, size_t YN = sizeof...(YZ) - 1>
    static auto execution(YZ&& ... yz) {
      auto tuple = std::forward_as_tuple(std::forward<YZ>(yz)...);
      // [0, size(TupleYZ)-1) = [0, YN)
      auto& parent1 = std::get<0>(std::get<YN>(tuple));
      return parent1.template wrap<RemoveEnd1<Parent2, Child, YN, ZN>>(std::move(tuple));
    };
  };

  template<typename Child, typename ... Z, size_t ZN = sizeof...(Z)>
  inline decltype(auto) wrap(Z&& ... z) {
    using Ctrl = traits::control_t<Child>;
    auto& parent1 = [&]() -> auto& {
      if constexpr (Ctrl::is_reversed) { return args.parent; } else { return parent; }
    }();
    auto& parent2 = [&]() -> auto& {
      if constexpr (Ctrl::is_reversed) { return parent; } else { return args.parent; }
    }();
    return parent2.template wrap<RemoveEnd2<Child, ZN>>(
      std::forward_as_tuple(parent1, std::forward<Z>(z)...)
    );
  }
};

template<typename Parent,
  std::enable_if_t<traits::is_coll_operator<Parent>::value>* = nullptr>
inline ConcatArgs<Parent> concat(Parent&& parent) {
  return {std::forward<Parent>(parent)};
}

template<typename Parent, typename Args,
  std::enable_if_t<Args::name == "concat">* = nullptr,
  std::enable_if_t<traits::is_coll_operator<Parent>::value>* = nullptr>
inline Concat<Parent, Args>
operator | (Parent&& parent, Args&& args) {
  return {std::forward<Parent>(parent), std::forward<Args>(args)};
}
} // namespace coll
