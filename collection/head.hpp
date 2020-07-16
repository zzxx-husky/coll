#pragma once

#include "base.hpp"
#include "reference.hpp"

namespace coll {
template<bool Ref>
struct HeadArgs {
  inline HeadArgs<true> ref() { return {}; }

  constexpr static bool take_ref = Ref;
};

template<typename Parent, typename Args>
struct Head {
  using InputType = typename Parent::OutputType;

  Parent parent;
  Args args;

  inline auto head() {
    auto ctrl = default_control();
    using ResultType = std::conditional_t<Args::take_ref,
      Reference<typename traits::remove_vr_t<InputType>>,
      std::optional<typename traits::remove_cvr_t<InputType>>
    >;
    ResultType result;
    parent.foreach(ctrl,
      [&](InputType elem) {
        result = std::forward<InputType>(elem);
        ctrl.break_now = true;
      });
    return result;
  }
};

inline HeadArgs<false> head() { return {}; }

template<typename Parent, bool Ref,
  std::enable_if_t<traits::is_collection<Parent>::value>* = nullptr>
inline auto operator | (const Parent& parent, HeadArgs<Ref> args) {
  return Head<Parent, HeadArgs<Ref>>{parent, std::move(args)}.head();
}
} // namespace coll

