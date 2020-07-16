#pragma once

#include "base.hpp"
#include "reference.hpp"
#include "windowed_elements.hpp"

namespace coll {
// Note(zzxx): May consider pushing `size` and `step` to the template arguements
// but not sure what the benefit is for doing so
template<bool CacheByRef>
struct WindowArgs {
  size_t size;
  size_t step;

  inline WindowArgs<true> cache_by_ref() {
    return {size, step};
  }

  template<typename InputType>
  using WindowType = WindowedElements<InputType, CacheByRef>;

  template<typename InputType>
  inline WindowType<InputType> create_window() { return {size, step}; }
};

template<typename Parent, typename Args>
struct Window {
  using InputType = typename Parent::OutputType;
  using WindowType = typename Args::template WindowType<InputType>;
  using OutputType = const WindowType&;


  Parent parent;
  Args args;

  template<typename Ctrl, typename ChildProc>
  inline void foreach(Ctrl& ctrl, ChildProc proc) {
    //
    // We do not know what the start of the last window will be if
    // we do not have a complete view of all the inputs, so no native reversion here.
    // E.g. for window(2, 2) on inputs [1, 2, 3, 4, 5], the outputs are [[1, 2], [3, 4]].
    // If a `reverse()` follows `window()`, the outputs will be [[3, 4], [1, 2]],
    // instead of [[5, 4], [3, 2]] or [[4, 5], [2, 3]].
    //
    // A special case that allows native reversion is window with step = 1,
    // where each input will be put in at least one window.
    //
    static_assert(!Ctrl::is_reversed, "Window does not support reverse iteration. "
      "Consider to use `with_buffer()` for the closest downstream `reverse()` operator.");

    WindowType w = args.template create_window<InputType>();
    parent.foreach(ctrl,
      [&](InputType elem) {
        if (w.emplace(std::forward<InputType>(elem))) {
          proc(w);
        }
      });
    if (w.shrink_remaining_elements()) {
      proc(w);
    }
  }
};

WindowArgs<false> window(size_t size, size_t step) { return {size, step}; }

WindowArgs<false> window(size_t size) { return {size, size}; }

template<typename Parent, bool CacheByRef,
  std::enable_if_t<traits::is_collection<Parent>::value>* = nullptr>
inline Window<Parent, WindowArgs<CacheByRef>>
operator | (const Parent& parent, WindowArgs<CacheByRef> args) {
  return {parent, std::move(args)};
}
} // namespace coll

