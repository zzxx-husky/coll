#pragma once

#include <memory>

#include "base.hpp"

namespace coll {
template<typename Input>
struct TraversalChildBase {
  virtual void start_child() = 0;
  virtual void process_child(Input) = 0;
  virtual void end_child() = 0;
  virtual bool break_now() = 0;
};

template<typename Output>
struct TraversalParentBase {
  virtual ~TraversalParentBase() = default;

  virtual void start() = 0;
  virtual void process() = 0;
  virtual void end() = 0;
  virtual TraversalParentBase<Output>* copy() const = 0;
  virtual void set_traversal_child(TraversalChildBase<Output>& child) = 0;
};

template<typename Exec, typename Output>
struct TraversalParent: public TraversalParentBase<Output> {
  Exec execution;

  template<typename ... X>
  TraversalParent(X&& ... x):
    execution(std::forward<X>(x)...) {
  }

  TraversalParent(const TraversalParent<Exec, Output>& t):
    execution(t.execution) {
  }

  TraversalParent(TraversalParent<Exec, Output>&& t):
    execution(std::move(t.execution)) {
  }

  void start() override {
    execution.start();
  }

  void process() override {
    execution.process();
  }

  void end() override {
    execution.end();
  }

  void set_traversal_child(TraversalChildBase<Output>& child) override {
    execution.traversal_child = &child;
  }

  TraversalParentBase<Output>* copy() const override {
    return new TraversalParent<Exec, Output>(*this);
  }
};

template<typename Output>
struct Traversal {
  using OutputType = Output;

  Traversal() = default;

  Traversal(std::unique_ptr<TraversalParentBase<Output>>&& parent):
    parent(std::move(parent)) {
  }

  Traversal(Traversal<Output>&& t):
    parent(std::move(t.parent)) {
  }

  Traversal(const Traversal<Output>& t):
    parent(t.parent->copy()) {
  }

  std::unique_ptr<TraversalParentBase<Output>> parent = nullptr;

  template<typename Child>
  class TraversalChild: public Child, public TraversalChildBase<Output> {
  public:
    template<typename ... X>
    TraversalChild(std::unique_ptr<TraversalParentBase<Output>>&& parent, X&& ... x):
      parent(std::move(parent)),
      Child(std::forward<X>(x) ...) {
      this->parent->set_traversal_child(*this);
    }

    TraversalChild(const TraversalChild<Child>& t):
      parent(t.parent->copy()),
      Child(t) {
      this->parent->set_traversal_child(*this);
    }

    TraversalChild(TraversalChild<Child>&& t):
      parent(std::move(t.parent)),
      Child(std::move(static_cast<Child&>(t))) {
      this->parent->set_traversal_child(*this);
    }

    std::unique_ptr<TraversalParentBase<Output>> parent = nullptr;

    inline void start() {
      parent->start();
    }

    inline void process() {
      parent->process();
    }

    inline void end() {
      parent->end();
    }

  private:
    void start_child() override {
      Child::start();
    }

    void process_child(Output e) override {
      Child::process(std::forward<Output>(e));
    }

    void end_child() override {
      Child::end();
    }

    bool break_now() override {
      return Child::control().break_now;
    }
  };

  template<ExecutionType ET, typename Child, typename ... X>
  inline decltype(auto) wrap(X&& ... x) {
    using Ctrl = traits::operator_control_t<Child>;
    static_assert(!Ctrl::is_reversed,
      "Partition operator does not support reversion. Use `with_buffer()` for the nearest `reverse`");
    if constexpr (ET == Execute) {
      return Child::template execute<TraversalChild<Child>>(
        std::unique_ptr<TraversalParentBase<Output>>(parent->copy()),
        std::forward<X>(x) ...);
    } else if constexpr (ET == Construct) {
      return Child::template construct<ExecutionType::Execute, TraversalChild<Child>>(
        std::unique_ptr<TraversalParentBase<Output>>(parent->copy()),
        std::forward<X>(x) ...);
    } else {
      return TraversalChild<Child>(
        std::unique_ptr<TraversalParentBase<Output>>(parent->copy()),
        std::forward<X>(x) ...);
    }
  }
};

template<typename Output>
struct TraversalExecution {
  TraversalChildBase<Output>* traversal_child = nullptr;

  auto_val(ctrl, default_control());

  inline void start() {
    if (!traversal_child) {
      throw std::runtime_error("TraversalChildBase is not set before use.");
    }
    traversal_child->start_child();
    if (traversal_child->break_now()) {
      ctrl.break_now = true;
    }
  }

  inline auto& control() {
    return ctrl;
  }

  inline void process(Output e) {
    traversal_child->process_child(std::forward<Output>(e));
    if (unlikely(traversal_child->break_now())) {
      ctrl.break_now = true;
    }
  }

  inline void end() {
    traversal_child->end_child();
  }

  template<ExecutionType ET, typename Exec, typename ... ArgT>
  static auto construct(ArgT&& ... args) {
    if constexpr (ET == Execute) {
      return TraversalParent<Exec, Output>(std::forward<ArgT>(args) ...);
    }
    // TODO(zzxx): handle ET == Object
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
  return Traversal<O>{
    std::unique_ptr<TraversalParentBase<O>>(new E(std::move(e)))
  };
}
} // namespace coll
