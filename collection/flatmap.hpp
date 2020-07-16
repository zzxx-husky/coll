#pragma once

#include "base.hpp"
#include "traits.hpp"

namespace coll {
template<typename M>
struct FlatmapArgs {
  M mapper;
};

template<typename Parent, typename M>
struct FlatmapIterable {
  using InputType = typename Parent::OutputType;
  using MapperResultType = typename traits::invocation<M, InputType>::result_t;
  using OutputType = typename traits::iterable<MapperResultType>::element_t;

  Parent parent;
  FlatmapArgs<M> args;

  template<typename Ctrl, typename ChildProc>
  inline void foreach(Ctrl& ctrl, ChildProc proc) {
    parent.foreach(ctrl,
      [=](InputType elem) {
        auto&& r = mapper(std::forward<InputType>(elem));
        if constexpr (Ctrl::is_reversed) {
          for (auto i = std::rbegin(r), e = std::rend(r);
               i != e && !ctrl.break_now; ++i) {
            proc(*i);
          }
        } else {
          for (auto i = std::begin(r), e = std::end(r);
               i != e && !ctrl.break_now; ++i) {
            proc(*i);
          }
        }
      });
  }
};

// For c-style string that ends with '\0'.
template<typename Parent, typename M>
struct FlatmapCStr {
  using InputType = typename Parent::OutputType;
  using MapperResultType = typename traits::invocation<M, InputType>::result_t;
  using OutputType = decltype(*std::declval<MapperResultType>());

  Parent parent;
  FlatmapArgs<M> args;

  template<typename Ctrl, typename ChildProc>
  inline void foreach(Ctrl& ctrl, ChildProc proc) {
    parent.foreach(ctrl,
      [=](InputType elem) {
        auto str = args.mapper(std::forward<InputType>(elem));
        if constexpr (Ctrl::is_reversed) {
          for (auto e = str;; ++e) {
            if (*e) {
              for (auto i = e; i != str && !ctrl.break_now;) {
                proc(*(--i));
              }
              break;
            }
          }
        } else {
          for (auto i = str; *i && !ctrl.break_now; ++i) {
            proc(*i);
          }
        }
      });
  }
};

template<typename Parent, typename M>
struct FlatmapCollection {
  using InputType = typename Parent::OutputType;
  using MapperResultType = typename traits::invocation<M, InputType>::result_t;
  using OutputType = typename traits::remove_cvr_t<MapperResultType>::OutputType;

  Parent parent;
  FlatmapArgs<M> args;

  template<typename Ctrl, typename ChildProc>
  inline void foreach(Ctrl& ctrl, ChildProc proc) {
    // Note: reversion is natively handled.
    parent.foreach(ctrl,
      [=](InputType elem) {
        auto&& collection = args.mapper(std::forward<InputType>(elem));
        collection.foreach(ctrl, [=](OutputType elem) {
          proc(std::forward<OutputType>(elem));
        });
      });
  }
};

template<typename M>
inline FlatmapArgs<M> flatmap(M mapper) { return {mapper}; }

inline auto flatten() {
  return flatmap([](auto&& e) -> auto&& {
    return std::forward<decltype(e)>(e);
  });
}

template<typename Parent, typename M,
  std::enable_if_t<traits::is_collection<Parent>::value>* = nullptr,
  typename MapperResult = typename traits::invocation<M, typename Parent::OutputType>::result_t,
  std::enable_if_t<traits::is_iterable<MapperResult>::value>* = nullptr>
inline FlatmapIterable<Parent, M> operator | (const Parent& parent, FlatmapArgs<M> args) {
  return {parent, std::move(args)};
}

template<typename Parent, typename M,
  std::enable_if_t<traits::is_collection<Parent>::value>* = nullptr,
  typename MapperResult = typename traits::invocation<M, typename Parent::OutputType>::result_t,
  std::enable_if_t<traits::is_c_str<MapperResult>::value>* = nullptr>
inline FlatmapCStr<Parent, M> operator | (const Parent& parent, FlatmapArgs<M> args) {
  return {parent, std::move(args)};
}

template<typename Parent, typename M,
  std::enable_if_t<traits::is_collection<Parent>::value>* = nullptr,
  typename MapperResult = typename traits::invocation<M, typename Parent::OutputType>::result_t,
  std::enable_if_t<traits::is_collection<MapperResult>::value>* = nullptr>
inline FlatmapCollection<Parent, M> operator | (const Parent& parent, FlatmapArgs<M> args) {
  return {parent, std::move(args)};
}
} // namespace coll
