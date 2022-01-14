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
  typename P = traits::remove_cvr_t<Parent>,
  std::enable_if_t<traits::is_pipe_operator<P>::value>* = nullptr>
inline ConcatArgs<P> concat(Parent&& parent) {
  return {std::forward<Parent>(parent)};
}

class Left {
private:
  Left() = default;
public:
  const static Left value;
};

inline const Left Left::value{};

class Right {
private:
  Right() = default;
public:
  const static Right value;
};

inline const Right Right::value{};

template<typename Parent, typename Args>
struct Concat {
  using OutputType = typename std::common_type_t<
    typename traits::remove_cvr_t<Parent>::OutputType,
    typename Args::ParentType::OutputType
  >;

  Parent parent;
  Args args;

  // [0, X) for Parent1, [X, Y) for Parent2, [Y + 1, Z) for Child
  template<typename Parent1, typename Parent2, typename Child,
           size_t XN, size_t YN>
  struct Execution : public Parent1, public Parent2, public Child {
    template<typename ... XYZ,
      size_t ZN = sizeof ... (XYZ) - XN - YN - 1>
    Execution(XYZ&& ... xyz): Execution(
      std::forward_as_tuple(xyz ...),
      std::make_index_sequence<XN>{},
      std::make_index_sequence<YN>{},
      std::make_index_sequence<ZN>{}) {
    }

    template<typename TupleXYZ, size_t ... XI, size_t ... YI, size_t ... ZI>
    Execution(TupleXYZ&& xyz, std::index_sequence<XI...>, std::index_sequence<YI...>, std::index_sequence<ZI...>):
      Parent1(std::get<XI>(xyz)..., static_cast<Child&>(*this)),
      Parent2(std::get<XN + YI>(xyz)..., static_cast<Child&>(*this)),
      Child  (std::get<XN + YN + 1 + ZI>(xyz)...) {
    }

    // This is used to run the execution.
    inline void process() {
      Parent1::process();
      Parent2::process();
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

  template<ExecutionType ET2, typename Parent2,
    ExecutionType ETChild, typename Child, size_t YN, size_t ZN>
  struct RemoveEnd1 {
    RemoveEnd1(Child& child):
      child(child),
      ctrl(child.ctrl) {
    }

    Child& child;
    typename traits::operator_control_t<Child>& ctrl;

    inline auto& control() {
      return ctrl;
    }

    inline void start() {}

    inline void process(OutputType e) {
      child.process(std::forward<OutputType>(e));
    }

    inline void end() {}

    template<ExecutionType ET1, typename Parent1, typename ... XYZ,
      size_t XN = sizeof...(XYZ) - YN - 1 - ZN>
    static decltype(auto) construct(XYZ&& ... xyz) {
      static_assert(ET1 != Construct && ET2 != Construct);
      if constexpr (ETChild == Construct) {
        constexpr ExecutionType ET = [&]() {
          if constexpr (ET1 == Execute && ET2 == Execute) {
            return ExecutionType::Execute;
          } else /* if constexpr (ET1 == Object || ET2 == Object) */ {
            return ExecutionType::Object;
          }
        }();
        return Child::template construct<ET, Execution<Parent1, Parent2, Child, XN, YN>>(
          std::forward<XYZ>(xyz)...
        );
      } else if constexpr (ETChild == Execute) {
        if constexpr (ET1 == Execute && ET2 == Execute) {
          return Child::template execute<Execution<Parent1, Parent2, Child, XN, YN>>(
            std::forward<XYZ>(xyz)...
          );
        } else /* if constexpr (ET1 == Object || ET2 == Object) */ {
          return Execution<Parent1, Parent2, Child, XN, YN>(
            std::forward<XYZ>(xyz)...
          );
        }
      }
    }
  };

  template<ExecutionType ETChild, typename Child, size_t ZN>
  struct RemoveEnd2 {
    RemoveEnd2(Child& child):
      child(child),
      ctrl(child.ctrl) {
    }

    Child& child;
    typename traits::operator_control_t<Child>& ctrl;

    inline auto& control() {
      return ctrl;
    }

    inline void start() {}

    inline void process(OutputType e) {
      child.process(std::forward<OutputType>(e));
    }

    inline void end() {}

    template<ExecutionType ET2, typename Parent2, typename ... YZ,
      size_t YN = sizeof...(YZ) - 1 - ZN>
    static auto construct(YZ&& ... yz) {
      // wrap parent1 which constructs RemoveEnd1
      auto& parent1 = std::get<YN>(std::tie(yz ...));
      return parent1.template wrap<ExecutionType::Construct,
        RemoveEnd1<ET2, Parent2, ETChild, Child, YN, ZN>>(std::forward<YZ>(yz) ...);
    };
  };

  template<ExecutionType ET,
    typename Child, typename ... Z, size_t ZN = sizeof...(Z)>
  inline decltype(auto) wrap(Z&& ... z) {
    using Ctrl = traits::operator_control_t<Child>;
    auto& parent1 = [&]() -> auto& {
      if constexpr (Ctrl::is_reversed) { return args.parent; } else { return parent; }
    }();
    auto& parent2 = [&]() -> auto& {
      if constexpr (Ctrl::is_reversed) { return parent; } else { return args.parent; }
    }();
    // wrap parent2 first which constructs a RemoveEnd2
    return parent2.template wrap<ExecutionType::Construct,
      RemoveEnd2<ET, Child, ZN>>(parent1, std::forward<Z>(z)...);
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
