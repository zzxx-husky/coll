#pragma once

#include "base.hpp"
#include "reference.hpp"

namespace coll {
template<bool Ref>
struct LastArgs {
  inline LastArgs<true> ref() { return {}; }

  constexpr static bool take_ref = Ref;
};

template<typename Parent, typename Args>
struct Last {
  using InputType = typename Parent::OutputType;

  Parent parent;
  Args args;

  inline auto last() {
    auto ctrl = default_control();
    using ResultType = std::conditional_t<Args::take_ref,
      Reference<typename traits::remove_vr_t<InputType>>,
      std::optional<typename traits::remove_cvr_t<InputType>>
    >;
    ResultType result;
    parent.foreach(ctrl,
      [&](InputType elem) {
        result = std::forward<InputType>(elem);
      });
    return result;
  }
};

inline LastArgs<false> last() { return {}; }

template<typename Parent, bool Ref,
  std::enable_if_t<traits::is_collection<Parent>::value>* = nullptr>
inline auto operator | (const Parent& parent, LastArgs<Ref> args) {
  return Last<Parent, LastArgs<Ref>>{parent, std::move(args)}.last();
}
} // namespace coll


