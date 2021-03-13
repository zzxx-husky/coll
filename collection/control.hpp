#pragma once

namespace coll {
template<bool Reverse>
struct Control {
  using ReverseType = Control<!Reverse>;
  using ForwardType = Control<false>;

  // used by child operator to notify parent to stop processing
  bool break_now = false;
  // whether to reversely iterate the elements
  constexpr static bool is_reversed = Reverse;

  // reverse the iteration order based on the current order.
  inline Control<!Reverse> reverse() {
    return {break_now};
  }

  // make the iteration order to be forward
  inline Control<false> forward() {
    return {break_now};
  }
};

inline Control<false> default_control() { return {}; }

using DefaultControl = decltype(default_control());
} // namespace coll
