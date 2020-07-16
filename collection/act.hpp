#pragma once

#include "base.hpp"

namespace coll {
struct act {};

template<typename Parent,
  std::enable_if_t<traits::is_collection<Parent>::value>* = nullptr>
inline void operator | (Parent parent, act) {
  auto ctrl = default_control();
  parent.foreach(ctrl, [](auto&&){});
}
} // namespace coll
