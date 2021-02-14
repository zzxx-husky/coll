#pragma once

#include "base.hpp"
#include "reference.hpp"
#include "windowed_elements.hpp"

namespace coll {
// Note(zzxx): May consider pushing `size` and `step` to the template arguements
// but not sure what the benefit is for doing so
template<bool CacheByRef>
struct WindowArgs {
  constexpr static std::string_view name = "window";

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

  template<typename Child>
  struct Proc : public Child {
    template<typename ...X>
    Proc(const Args& args, X&& ... x):
      args(args),
      Child(std::forward<X>(x)...) {
    }

    Args args;
    auto_val(w, args.template create_window<InputType>());

    inline void process(InputType e) {
      if (w.emplace(std::forward<InputType>(e))) {
        Child::process(w);
      }
    }

    inline void end() {
      if (w.shrink_remaining_elements()) {
        Child::process(w);
      }
      Child::end();
    }
  };

  template<typename Child, typename ... X>
  inline decltype(auto) wrap(X&& ... x) {
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
    using Ctrl = traits::control_t<Child>;
    static_assert(!Ctrl::is_reversed, "Window does not support reverse iteration. "
      "Consider to use `with_buffer()` for the closest downstream `reverse()` operator.");

    return parent.template wrap<Proc<Child>, Args&, X...>(
      args, std::forward<X>(x)...
    );
  }
};

WindowArgs<false> window(size_t size, size_t step) { return {size, step}; }

WindowArgs<false> window(size_t size) { return {size, size}; }

template<typename Parent, typename Args,
  std::enable_if_t<Args::name == "window">* = nullptr,
  std::enable_if_t<traits::is_pipe_operator<Parent>::value>* = nullptr>
inline Window<Parent, Args>
operator | (Parent&& parent, Args&& args) {
  return {std::forward<Parent>(parent), std::forward<Args>(args)};
}
} // namespace coll

