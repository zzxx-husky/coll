#pragma once

#include <memory>

#include "base.hpp"

namespace coll {
template<typename Input>
struct TraversalChildBase {
  virtual void child_start() = 0;
  virtual void child_process(Input) = 0;
  virtual void child_end() = 0;
  virtual bool child_break_now() = 0;
  virtual ~TraversalChildBase() = default;
};

template<typename Output, typename Ts>
struct TraversalParentBase;

template<typename Output, typename ... T>
struct TraversalParentBase<Output, Triggers<T ...>>: virtual public T ... {
  using TriggersType = Triggers<T ...>;

  virtual ~TraversalParentBase() = default;

  using T::run ...;

  virtual void start() = 0;
  virtual void end() = 0;
  virtual TraversalParentBase<Output, Triggers<T ...>>* copy() const = 0;
  virtual void set_traversal_child(TraversalChildBase<Output>* child) = 0;
};

template<typename Exec, typename Output>
struct TraversalParent:
  public TraversalParentBase<Output, typename Exec::TriggersType>,
  public VirtualTriggerProxies<Exec, typename Exec::TriggersType> {

  using TriggersType = typename Exec::TriggersType;
  using VirtualTriggerProxies<Exec, typename Exec::TriggersType>::run;

  Exec exec;

  template<typename ... X>
  TraversalParent(X&& ... x):
    exec(std::forward<X>(x)...) {
  }

  TraversalParent(const TraversalParent<Exec, Output>& t):
    exec(t.exec) {
  }

  TraversalParent(TraversalParent<Exec, Output>&& t):
    exec(std::move(t.exec)) {
  }

  Exec& execution() override {
    return exec;
  }

  void start() override {
    exec.start();
  }

  void end() override {
    exec.end();
  }

  void set_traversal_child(TraversalChildBase<Output>* child) override {
    exec.set_traversal_child(child);
  }

  TraversalParentBase<Output, typename Exec::TriggersType>* copy() const override {
    return new TraversalParent<Exec, Output>(*this);
  }
};

template<typename Output, typename Ts = Triggers<Run<>>>
struct Traversal {
  using OutputType = Output;

  Traversal() = default;

  Traversal(std::unique_ptr<TraversalParentBase<Output, Ts>>&& parent):
    parent(std::move(parent)) {
  }

  Traversal(Traversal<Output, Ts>&& t):
    parent(std::move(t.parent)) {
  }

  Traversal(const Traversal<Output, Ts>& t):
    parent(t.parent->copy()) {
  }

  std::unique_ptr<TraversalParentBase<Output, Ts>> parent = nullptr;

  template<typename Child>
  class TraversalChild:
    // to inherit the processing of child
    public Child,
    // to provide an interface to parent to connect with child
    public TraversalChildBase<Output>,
    // to provide the same triggers as the ones in parent
    public TriggerProxies<TraversalParentBase<Output, Ts>, Ts> {
  public:
    using TriggersType = Ts;

    template<typename ... X>
    TraversalChild(std::unique_ptr<TraversalParentBase<Output, Ts>>&& parent, X&& ... x):
      Child(std::forward<X>(x) ...),
      parent(std::move(parent)) {
    }

    TraversalChild(const TraversalChild<Child>& t):
      Child(t),
      parent(t.parent->copy()) {
    }

    TraversalChild(TraversalChild<Child>&& t):
      Child(std::move(static_cast<Child&>(t))),
      parent(std::move(t.parent)) {
    }

    std::unique_ptr<TraversalParentBase<Output, Ts>> parent = nullptr;

    // override TriggerProxies<TraversalParentBase<Output, Ts>, Ts>::execution
    TraversalParentBase<Output, Ts>& execution() override {
      return *parent;
    }

    inline void start() {
      parent->set_traversal_child(static_cast<TraversalChildBase<Output>*>(this));
      parent->start();
    }

    inline void end() {
      parent->end();
    }

    void child_start() override {
      Child::start();
    }

    void child_process(Output e) override {
      Child::process(std::forward<Output>(e));
    }

    void child_end() override {
      Child::end();
    }

    bool child_break_now() override {
      return Child::control().break_now;
    }
  };

  template<ExecutionType ET, typename Child, typename ... X>
  inline decltype(auto) wrap(X&& ... x) {
    using Ctrl = traits::operator_control_t<Child>;
    static_assert(!Ctrl::is_reversed,
      "Partition operator does not support reversion. Use `with_buffer()` for the nearest `reverse`");
    constexpr ExecutionType SelfET =
      Ts::has_non_empty_run_trigger ? ExecutionType::Object : ExecutionType::Execute;
    if constexpr (ET == Construct) {
      return Child::template construct<SelfET, TraversalChild<Child>>(
        std::unique_ptr<TraversalParentBase<Output, Ts>>(parent->copy()),
        std::forward<X>(x) ...);
    } else if constexpr (ET == Object || SelfET == Object) {
      return TraversalChild<Child>(
        std::unique_ptr<TraversalParentBase<Output, Ts>>(parent->copy()),
        std::forward<X>(x) ...);
    } else {
      return Child::template execute<TraversalChild<Child>>(
        std::unique_ptr<TraversalParentBase<Output, Ts>>(parent->copy()),
        std::forward<X>(x) ...);
    }
  }
};

template<typename Output>
struct TraversalExecution {
  TraversalChildBase<Output>* traversal_child = nullptr;

  auto_val(ctrl, default_control());

  inline void set_traversal_child(TraversalChildBase<Output>* child) {
    this->traversal_child = child;
  }

  inline void start() {
    if (!traversal_child) {
      throw std::runtime_error("TraversalChildBase is not set before use.");
    }
    traversal_child->child_start();
    if (traversal_child->child_break_now()) {
      ctrl.break_now = true;
    }
  }

  inline auto& control() {
    return ctrl;
  }

  inline void process(Output e) {
    traversal_child->child_process(std::forward<Output>(e));
    if (unlikely(traversal_child->child_break_now())) {
      ctrl.break_now = true;
    }
  }

  inline void end() {
    traversal_child->child_end();
  }

  template<ExecutionType ET, typename Exec, typename ... ArgT>
  static auto construct(ArgT&& ... args) {
    if constexpr (ET == Execute) {
      return TraversalParent<Exec, Output>(std::forward<ArgT>(args) ...);
    } else /* if constexpr (ET == Object) */ {
      return TraversalParent<Exec, Output>(std::forward<ArgT>(args) ...);
    }
  }
};

struct TraversalArgs {};

inline TraversalArgs to_traversal() {
  return {};
}

template<typename Parent,
  typename P = traits::remove_cvr_t<Parent>,
  std::enable_if_t<traits::is_pipe_operator<P>::value>* = nullptr>
inline auto operator | (Parent&& parent, TraversalArgs) {
  using O = typename P::OutputType;
  auto e = parent.template wrap<ExecutionType::Construct, TraversalExecution<O>>();
  using E = decltype(e);
  return Traversal<O, typename E::TriggersType>{
    std::unique_ptr<TraversalParentBase<O, typename E::TriggersType>>(new E(std::move(e)))
  };
}
} // namespace coll
