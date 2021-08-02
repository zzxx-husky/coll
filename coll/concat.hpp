#pragma once

#include "base.hpp"

namespace coll {
struct ConcatArgsTag {};

template<typename Parent>
struct ConcatArgs {
  using TagType = ConcatArgsTag;

  using ParentType = Parent;

  Parent parent;
};

template<typename Parent,
  std::enable_if_t<traits::is_pipe_operator<Parent>::value>* = nullptr>
inline ConcatArgs<Parent> concat(Parent&& parent) {
  return {std::forward<Parent>(parent)};
}

class Left {
  Left() = default;
public:
  const static Left value;
};

class Right {
  Right() = default;
public:
  const static Right value;
};

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
  struct Execution : public Parent1, public Parent2, public Child {
    template<typename TupleXYZ>
    Execution(TupleXYZ&& xyz): Execution(
      std::forward<TupleXYZ>(xyz),
      std::make_index_sequence<XN>{},
      std::make_index_sequence<YN>{},
      std::make_index_sequence<ZN>{}) {
    }

    template<typename TupleXYZ, size_t ... XI, size_t ... YI, size_t ... ZI>
    Execution(TupleXYZ&& xyz, std::index_sequence<XI...>, std::index_sequence<YI...>, std::index_sequence<ZI...>):
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

    inline void start() {
      if constexpr (traits::execution_has_start<Parent1>::value) {
        Parent1::start();
      }
      if constexpr (traits::execution_has_start<Parent2>::value) {
        Parent2::start();
      }
      Child::start();
    }

    inline void end() {
      if constexpr (traits::execution_has_end<Parent1>::value) {
        Parent1::end();
      }
      if constexpr (traits::execution_has_end<Parent2>::value) {
        Parent2::end();
      }
      Child::end();
    }

    inline decltype(auto) result() {
      if constexpr (traits::execution_has_result<Child>::value) {
        return Child::result();
      }
    }
  };

  template<typename Parent2, typename Child, size_t YN, size_t ZN>
  struct RemoveEnd1 {
    RemoveEnd1(Child& child):
      child(child),
      control(child.control) {
    }

    Child& child;
    typename traits::operator_control_t<Child>& control;

    inline void process(OutputType e) {
      child.process(std::forward<OutputType>(e));
    }

    inline void end() {}

    constexpr static ExecutionType execution_type = Construct;

    template<typename Parent1, typename ... XYZ, size_t XN = sizeof...(XYZ) - 1>
    static auto execute(XYZ&& ... xyz) {
      return Child::template execute<Execution<Parent1, Parent2, Child, XN, YN, ZN>>(
        std::forward_as_tuple(std::forward<XYZ>(xyz)...)
      );
    };
  };

  template<typename Child, size_t ZN>
  struct RemoveEnd2 {
    RemoveEnd2(Child& child):
      child(child),
      control(child.control) {
    }

    Child& child;
    typename traits::operator_control_t<Child>& control;

    inline void process(OutputType e) {
      child.process(std::forward<OutputType>(e));
    }

    inline void end() {}

    constexpr static ExecutionType execution_type = Construct;

    template<typename Parent2, typename ... YZ, size_t YN = sizeof...(YZ) - 1>
    static auto execute(YZ&& ... yz) {
      auto tuple = std::forward_as_tuple(std::forward<YZ>(yz)...);
      // [0, size(TupleYZ)-1) = [0, YN)
      auto& parent1 = std::get<0>(std::get<YN>(tuple));
      return parent1.template wrap<RemoveEnd1<Parent2, Child, YN, ZN>>(std::move(tuple));
    };
  };

  template<typename Child, typename ... Z, size_t ZN = sizeof...(Z)>
  inline decltype(auto) wrap(Z&& ... z) {
    using Ctrl = traits::operator_control_t<Child>;
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

template<typename Parent, typename Args,
  typename P = traits::remove_cvr_t<Parent>,
  typename A = traits::remove_cvr_t<Args>,
  std::enable_if_t<std::is_same<typename A::TagType, ConcatArgsTag>::value>* = nullptr,
  std::enable_if_t<traits::is_pipe_operator<P>::value>* = nullptr>
inline Concat<P, A>
operator | (Parent&& parent, Args&& args) {
  return {std::forward<Parent>(parent), std::forward<Args>(args)};
}
} // namespace coll
