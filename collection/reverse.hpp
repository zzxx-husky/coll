#pragma once

#include <vector>

#include "base.hpp"
#include "reference.hpp"
#include "utils.hpp"

namespace coll {
template<bool CacheByRef>
struct ReverseWithBufferArgs {
  constexpr static bool reverse_with_buffer = true;
  constexpr static bool is_cache_by_ref = CacheByRef;

  inline ReverseWithBufferArgs<true> cache_by_ref() { return {}; }
};

struct ReverseArgs {
  constexpr static bool reverse_with_buffer = false;
  constexpr static bool is_cache_by_ref = false;

  inline ReverseWithBufferArgs<false> with_buffer() { return {}; }
};

template<typename Parent, typename Args>
struct Reverse {
  using InputType = typename Parent::OutputType;
  using OutputType = std::conditional_t<Args::reverse_with_buffer,
    // since we are using buffer, we should output refernce
    std::conditional_t<Args::is_cache_by_ref,
      traits::remove_vr_t<InputType>&,
      traits::remove_cvr_t<InputType>&
    >,
    InputType
  >;
      
  Parent parent;
  Args args;

  template<typename Ctrl, typename ChildProc>
  inline void foreach(Ctrl& ctrl, ChildProc proc) {
    if constexpr (Args::reverse_with_buffer) {
      // reverse but with buffer
      using ElemType = std::conditional_t<Args::is_cache_by_ref,
        Reference<traits::remove_vr_t<InputType>>,
        traits::remove_cvr_t<InputType>
      >;
      if constexpr (Ctrl::is_reversed) {
        // by doing so we can output reference to the child
        // this looks silly but we have to do it in this way because we do not know Ctrl::is_reversed in advance
        ElemType elem;
        parent.foreach(ctrl, [&](InputType e) {
          elem = std::forward<InputType>(e);
          if constexpr (Args::is_cache_by_ref) {
            proc(*elem);
          } else {
            proc(elem);
          }
        });
      } else {
        std::vector<ElemType> elems;
        auto new_ctrl = ctrl;
        parent.foreach(new_ctrl,
          [&](InputType elem) {
            elems.emplace_back(std::forward<InputType>(elem));
          });
        for (auto i = std::rbegin(elems), e = std::rend(elems);
             i != e && !ctrl.break_now; ++i) {
          if constexpr (Args::is_cache_by_ref) {
            proc(**i);
          } else {
            proc(*i);
          }
        }
      }
    } else {
      auto new_ctrl = ctrl.reverse();
      // no matter Ctrl::is_reversed or not, push reverse upwards
      parent.foreach(new_ctrl,
        [&](InputType elem) {
          if (unlikely(ctrl.break_now)) {
            new_ctrl.break_now = true;
          } else {
            proc(std::forward<InputType>(elem));
          }
        });
    }
  }
};

ReverseArgs reverse() { return {}; }

template<typename Parent,
  std::enable_if_t<traits::is_collection<Parent>::value>* = nullptr>
inline Reverse<Parent, ReverseArgs>
operator | (const Parent& parent, ReverseArgs args) {
  return {parent, std::move(args)};
}

template<typename Parent, bool CacheByRef,
  std::enable_if_t<traits::is_collection<Parent>::value>* = nullptr>
inline Reverse<Parent, ReverseWithBufferArgs<CacheByRef>>
operator | (const Parent& parent, ReverseWithBufferArgs<CacheByRef> args) {
  return {parent, std::move(args)};
}
} // namespace coll

