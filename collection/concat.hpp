#pragma once

#include "base.hpp"

namespace coll {
template<typename RParent>
struct ConcatArgs {
  RParent rparent;
};

template<typename LParent, typename RParent>
struct Concat {
  using OutputType = typename std::common_type_t<
    typename LParent::OutputType,
    typename RParent::OutputType
  >;

  LParent lparent;
  RParent rparent;

  template<typename Ctrl, typename ChildProc>
  inline void foreach(Ctrl& ctrl, ChildProc proc) {
    if constexpr (Ctrl::is_reversed) {
      rparent.foreach(ctrl, proc);
      lparent.foreach(ctrl, proc);
    } else {
      lparent.foreach(ctrl, proc);
      rparent.foreach(ctrl, proc);
    }
  }
};

template<typename RParent>
inline ConcatArgs<RParent> concat(RParent rparent) { return {rparent}; }

template<typename LParent, typename RParent,
  std::enable_if_t<traits::is_collection<LParent>::value>* = nullptr,
  std::enable_if_t<traits::is_collection<RParent>::value>* = nullptr>
inline Concat<LParent, RParent> operator | (const LParent& lparent, const ConcatArgs<RParent>& args) {
  return {lparent, args.rparent};
}
} // namespace coll
