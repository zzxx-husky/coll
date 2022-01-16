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
    size_t XN, size_t YN, typename Triggers = typename MergedTriggers<
      typename Parent1::TriggersType, typename Parent2::TriggersType>::type>
  struct Execution: public Child {
    using TriggersType = typename MergedTriggers<Triggers,
      typename MergedTriggers<
        typename MapTriggers<typename Parent1::TriggersType, Left>::type,
        typename MapTriggers<typename Parent2::TriggersType, Right>::type
      >::type>::type;

    Parent1 parent1;
    Parent2 parent2;

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
      parent1(std::get<XI>(xyz)...),
      parent2(std::get<XN + YI>(xyz)...),
      Child  (std::get<XN + YN + 1 + ZI>(xyz)...) {
      parent1.remove_end_child = this;
      parent2.remove_end_child = this;
    }

    Execution(const Execution<Parent1, Parent2, Child, XN, YN>& e):
      parent1(e.parent1),
      parent2(e.parent2),
      Child(e) {
      parent1.remove_end_child = this;
      parent2.remove_end_child = this;
    }

    Execution(Execution<Parent1, Parent2, Child, XN, YN>&& e):
      parent1(std::move(e.parent1)),
      parent2(std::move(e.parent2)),
      Child(std::move(static_cast<Child&>(e))) {
      parent1.remove_end_child = this;
      parent2.remove_end_child = this;
    }

    inline auto& control() {
      return Child::control();
    }

    template<typename ... A>
    inline void run(A&& ... args) {
      if constexpr (traits::execution_has_run<Parent1, A ...>::value) {
        parent1.run(args ...);
      }
      if constexpr (traits::execution_has_run<Parent2, A ...>::value) {
        parent2.run(args ...);
      }
    }

    // Invoke the first parent only
    template<typename ... ArgT>
    inline void run(Left, ArgT&& ... args) {
      parent1.run(std::forward<ArgT>(args) ...);
    }

    // Invoke the second parent only
    template<typename ... ArgT>
    inline void run(Right, ArgT&& ... args) {
      parent2.run(std::forward<ArgT>(args) ...);
    }

    inline void start() {
      parent1.start();
      parent2.start();
      Child::start();
    }

    inline void end() {
      parent1.end();
      parent2.end();
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
    Child* remove_end_child = nullptr;

    inline auto& control() {
      return remove_end_child->control();
    }

    inline void start() {}

    inline void process(OutputType e) {
      remove_end_child->process(std::forward<OutputType>(e));
    }

    inline void end() {}

    template<ExecutionType ET1, typename Parent1, typename ... XYZ,
      size_t XN = sizeof...(XYZ) - YN - 1 - ZN>
    static decltype(auto) construct(XYZ&& ... xyz) {
      static_assert(ET1 != Construct && ET2 != Construct);
      if constexpr (ETChild == Construct) {
        constexpr ExecutionType ET = ET1 == Execute && ET2 == Execute
            ? ExecutionType::Execute
            : ExecutionType::Object;
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
    Child* remove_end_child = nullptr;

    inline auto& control() {
      return remove_end_child->control();
    }

    inline void start() {}

    inline void process(OutputType e) {
      remove_end_child->process(std::forward<OutputType>(e));
    }

    inline void end() {}

    // y... + parent1 + z...
    // y... are the arguments to construct Parent2
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
    // parent1 is used for wrapping the Parent1 part
    // z... are the arguments to construct Child
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
