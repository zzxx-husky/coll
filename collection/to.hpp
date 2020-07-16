#pragma once

namespace coll {
template<typename ContainerVRB, bool CopyOrMove>
struct ToArgs {
  // 1. a container copy;
  // 2. a reference to an existing container
  // 3. a builder that constructs a new container
  ContainerVRB vrb;
  size_t reserve_size = 0;

  // used by users

  inline auto& reserve(size_t size) {
    reserve_size = size;
    return *this;
  }

  inline ToArgs<ContainerVRB, false> by_move() {
    return {std::forward<ContainerVRB>(vrb), reserve_size};
  }

  // used by operators

  constexpr static bool ins_by_copy = CopyOrMove;
  constexpr static bool ins_by_move = !CopyOrMove;

  constexpr static bool is_container_ref =
    // Note: no 'const T&' appear here, only T&
    std::is_lvalue_reference<ContainerVRB>::value;

  // https://stackoverflow.com/questions/55301615/how-to-decide-constexpr-to-return-a-reference-or-not
  template<typename Input, typename Elem = traits::remove_cvr_t<Input>>
  decltype(auto) get_container() {
    if constexpr (traits::is_builder<ContainerVRB, Elem>::value) {
      // Builder
      return vrb(Type<Elem>{});
    } else {
      // Reference or Value
      return vrb;
    }
  }

  template<typename Input>
  using ContainerType = decltype(std::declval<
    ToArgs<ContainerVRB, CopyOrMove>
  >().template get_container<Input>());
};

template<typename ContainerVB>
inline ToArgs<ContainerVB, true> to(ContainerVB&& vb) {
  // std::move to handle const T&
  return {std::move(vb)};
}

template<typename ContainerR>
inline ToArgs<ContainerR&, true> to(ContainerR& container_ref) {
  return {container_ref};
}

template<typename ContainerV>
inline auto to() {
  return to([=](auto&&) { return ContainerV{}; });
}

template<template<typename ...> class ContainerT>
inline auto to() {
  return to([](auto&& type) {
    return ContainerT<typename traits::remove_cvr_t<decltype(type)>::type>();
  });
}

template<typename Parent>
struct To {
  using InputType = typename Parent::OutputType;

  Parent parent;

  template<typename Args,
    std::enable_if_t<!Args::is_container_ref>* = nullptr>
  inline auto to(Args args) {
    auto container = args.template get_container<InputType>();
    if (args.reserve_size) {
      container_utils::reserve(container, args.reserve_size);
    }
    auto ctrl = default_control();
    if constexpr (Args::ins_by_copy) {
      parent.foreach(ctrl,
        [&](InputType elem) {
          container_utils::insert(container, std::forward<InputType>(elem));
        });
    } else {
      parent.foreach(ctrl,
        [&](InputType elem) {
          container_utils::insert(container, std::move(elem));
        });
    }
    return container;
  }

  template<typename Args,
    std::enable_if_t<Args::is_container_ref>* = nullptr>
  inline void to(Args args) {
    auto& container = args.template get_container<InputType>();
    auto ctrl = default_control();
    if constexpr (Args::ins_by_copy) {
      parent.foreach(ctrl,
        [&](InputType elem) {
          container_utils::insert(container, std::forward<InputType>(elem));
        });
    } else {
      parent.foreach(ctrl,
        [&](InputType elem) {
          container_utils::insert(container, std::move(elem));
        });
    }
  }
};

template<typename Parent, typename ContainerVRB, bool CopyOrMove,
  std::enable_if_t<traits::is_collection<Parent>::value>* = nullptr>
inline auto operator | (const Parent& parent, ToArgs<ContainerVRB, CopyOrMove> args) {
  // Note: return value can be void
  return To<Parent>{parent}.to(std::move(args));
}
} // namespace coll
