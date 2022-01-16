#pragma once

#include <type_traits>

#include "traits.hpp"

namespace coll {
template<typename ... ArgT>
struct Run {
  virtual void run(ArgT ...) = 0;
};

template<typename ... T>
struct Triggers {
  template<typename ...>
  struct HasNonEmptyRunTrigger {
    inline static constexpr bool value = false;
  };

  template<typename ... ArgT, typename ... X>
  struct HasNonEmptyRunTrigger<Run<ArgT ...>, X ...> {
    inline static constexpr bool value = true;
  };

  template<typename ... X>
  struct HasNonEmptyRunTrigger<Run<>, X ...>: public HasNonEmptyRunTrigger<X ...> {};

  inline static constexpr bool has_non_empty_run_trigger = HasNonEmptyRunTrigger<T ...>::value;
};

template<typename Exec>
struct TriggerProxyBase {
  virtual Exec& execution() = 0;
};

template<typename Exec, typename T>
struct VirtualTriggerProxy;

template<typename Exec, typename ... ArgT>
struct VirtualTriggerProxy<Exec, Run<ArgT ...>>:
  virtual public Run<ArgT ...>,
  virtual public TriggerProxyBase<Exec> {

  void run(ArgT ... args) override {
    this->execution().run(std::forward<ArgT>(args) ...);
  }
};

template<typename Exec, typename Ts>
struct VirtualTriggerProxies;

template<typename Exec, typename ... T>
struct VirtualTriggerProxies<Exec, Triggers<T ...>>:
  public VirtualTriggerProxy<Exec, T> ... {
  using VirtualTriggerProxy<Exec, T>::run ...;
};

template<typename Exec, typename T>
struct TriggerProxy;

template<typename Exec, typename ... ArgT>
struct TriggerProxy<Exec, Run<ArgT ...>>:
  virtual public TriggerProxyBase<Exec> {

  inline void run(ArgT ... args) {
    this->execution().run(std::forward<ArgT>(args) ...);
  }
};

template<typename Exec, typename Ts>
struct TriggerProxies;

template<typename Exec, typename ... T>
struct TriggerProxies<Exec, Triggers<T ...>>: public TriggerProxy<Exec, T> ... {
  using TriggerProxy<Exec, T>::run ...;
};

namespace triggers {
template<typename Ts1, typename Ts2, typename ... O>
struct Merge;

template<typename ... O>
struct Merge<Triggers<>, Triggers<>, O ...> {
  using type = Triggers<O ...>;
};

template<typename T, typename ... T1, typename ... T2, typename ... O>
struct Merge<Triggers<T, T1 ...>, Triggers<T, T2 ...>, O ...>:
  public Merge<Triggers<T1 ...>, Triggers<T2 ...>, O..., T> {
};

template<typename T1, typename T2, typename ... Ts1, typename ... Ts2, typename ... O>
struct Merge<Triggers<T1, Ts1 ...>, Triggers<T2, Ts2 ...>, O ...>:
  public std::conditional_t<
    traits::type_name<T1>::name() < traits::type_name<T2>::name(),
    Merge<Triggers<Ts1 ...>, Triggers<T2, Ts2 ...>, O..., T1>,
    Merge<Triggers<T1, Ts1 ...>, Triggers<Ts2 ...>, O..., T2>> {
};

template<typename ... T1, typename ... O>
struct Merge<Triggers<T1 ...>, Triggers<>, O ...>:
  public Merge<Triggers<>, Triggers<>, O..., T1 ...> {
};

template<typename ... T2, typename ... O>
struct Merge<Triggers<>, Triggers<T2 ...>, O ...>:
  public Merge<Triggers<>, Triggers<>, O..., T2 ...> {
};

template<typename Ts, typename ... O>
struct RemoveEmptyRunTrigger;

template<typename F, typename ... Ts, typename ... O>
struct RemoveEmptyRunTrigger<Triggers<F, Ts ...>, O ...>:
  public RemoveEmptyRunTrigger<Triggers<Ts ...>, O..., F> {
};

template<typename ... Ts, typename ... O>
struct RemoveEmptyRunTrigger<Triggers<Run<>, Ts ...>, O ...>:
  public RemoveEmptyRunTrigger<Triggers<Ts ...>, O...> {
};

template<typename ... O>
struct RemoveEmptyRunTrigger<Triggers<>, O ...> {
  using type = Triggers<O ...>;
};
} // namespace triggers

template<typename Ts1, typename Ts2>
struct MergedTriggers {
  using type = typename triggers::Merge<Ts1, Ts2>::type;
};

template<typename Ts>
struct RemoveEmptyRunTrigger {
  using type = typename triggers::RemoveEmptyRunTrigger<Ts>::type;
};

template<typename X, typename T>
struct MapTrigger;

template<typename ...ArgT, typename T>
struct MapTrigger<Run<ArgT ...>, T> {
  using type = Run<T, ArgT ...>;
};

template<typename Ts, typename T>
struct MapTriggers;

template<typename ... Ts, typename T>
struct MapTriggers<Triggers<Ts ...>, T> {
  using type = Triggers<typename MapTrigger<Ts, T>::type ...>;
};
} // namespace coll
