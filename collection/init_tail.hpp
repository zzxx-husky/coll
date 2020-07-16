#pragma once

#include <optional>

#include "base.hpp"
#include "utils.hpp"

namespace coll {
template<bool InitOrTail, bool CacheByRef>
struct InitTailArgs {
  inline InitTailArgs<InitOrTail, true> cache_by_ref() { return {}; }

  static constexpr bool is_init = InitOrTail;
  static constexpr bool is_tail = !InitOrTail;
  static constexpr bool is_cache_by_ref = CacheByRef;
};

// return all elements except the last one
template<typename Parent, typename Args>
struct InitTail {
  using InputType = typename Parent::OutputType;
  using OutputType = InputType;

  Parent parent;
  Args args;

  template<typename Ctrl, typename ChildProc>
  inline void init(Ctrl& ctrl, ChildProc proc) {
    using CacheType = std::conditional_t<Args::is_cache_by_ref,
      Reference<traits::remove_vr_t<InputType>>,
      std::optional<traits::remove_cvr_t<InputType>>
    >;
    CacheType prev_elem;
    parent.foreach(ctrl,
      [&](InputType elem) {
        if (likely(bool(prev_elem))) {
          proc(*prev_elem);
        }
        prev_elem = elem;
      });
  }

  template<typename Ctrl, typename ChildProc>
  inline void tail(Ctrl& ctrl, ChildProc proc) {
    bool skipped_head = false;
    parent.foreach(ctrl,
      [&](InputType elem) {
        if (likely(skipped_head)) {
          proc(std::forward<InputType>(elem));
        } else {
          skipped_head = true;
        }
      });
  }

  template<typename Ctrl, typename ChildProc>
  inline void foreach(Ctrl& ctrl, ChildProc proc) {
    if constexpr (Args::is_init != Ctrl::is_reversed) {
      init(ctrl, std::forward<ChildProc>(proc));
    } else {
      tail(ctrl, std::forward<ChildProc>(proc));
    }
  }
};

inline InitTailArgs<true, false> init() { return {}; }
inline InitTailArgs<false, false> tail() { return {}; }

template<typename Parent, bool InitOrTail, bool CacheByRef,
  std::enable_if_t<traits::is_collection<Parent>::value>* = nullptr>
inline InitTail<Parent, InitTailArgs<InitOrTail, CacheByRef>>
operator | (const Parent& parent, InitTailArgs<InitOrTail, CacheByRef> args) {
  return {parent, std::move(args)};
}
} // namespace coll

