#pragma once

#include "base.hpp"

namespace coll {
template<typename Input>
struct PlaceHolder {
  using OutputType = Input;

  template<typename Child, typename ... X>
  inline decltype(auto) wrap(X&& ... x) {
    if constexpr (Child::execution_type == ConstructExecution) {
      return Child::template execution<Child>(std::forward<X>(x)...);
    } else {
      return Child(std::forward<X>(x)...);
    }
  }
};

template<typename Input>
inline PlaceHolder<Input> place_holder() { return {}; }
} // namespace coll
