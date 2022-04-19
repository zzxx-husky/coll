#pragma once
#if ENABLE_PARALLEL

#include <deque>
#include <vector>

#include "message_codes.hpp"

namespace coll {
namespace shuffle {
namespace details {
template<typename I>
class RandomAssign {
public:
  void initialize(zaf::ActorGroup& group, std::vector<zaf::Actor>& executors) {
    forwarder = group.create_scoped_actor<zaf::ActorBehaviorX>();
    this->executors = executors;
    for (auto& e : executors) {
      forwarder->send(e, codes::Downstream, forwarder->get_self_actor());
    }
  }

  template<typename U>
  inline void dispatch(U&& elem) {
    forwarder->send(executors[rand() % executors.size()], codes::Data, std::forward<U>(elem));
  }

  inline void clear() {
    forwarder = nullptr;
  }

  void terminate() {
    for (auto& e : executors) {
      forwarder->send(e, codes::Termination);
    }
  }

  inline bool receive(zaf::MessageHandlers& handlers, bool non_blocking) {
    return forwarder->receive_once(handlers, non_blocking);
  }

private:
  std::vector<zaf::Actor> executors;
  zaf::ScopedActor<zaf::ActorBehaviorX> forwarder;
};

template<typename I, typename KeyBy>
class Partition {
public:
  using KeyType = traits::remove_cvr_t<typename traits::invocation<KeyBy, I>::result_t>;

  Partition(const KeyBy& key_by):
    key_by(key_by) {
  }

  void initialize(zaf::ActorGroup& group, std::vector<zaf::Actor>& executors) {
    forwarder = group.create_scoped_actor<zaf::ActorBehaviorX>();
    this->executors = executors;
    for (auto& e : executors) {
      forwarder->send(e, codes::Downstream, forwarder->get_self_actor());
    }
  }

  template<typename U>
  inline void dispatch(U&& elem) {
    auto key_hash = hasher(key_by(elem));
    forwarder->send(executors[key_hash % executors.size()], codes::Data, std::forward<U>(elem));
  }

  inline void clear() {
    forwarder = nullptr;
  }

  void terminate() {
    for (auto& e : executors) {
      forwarder->send(e, codes::Termination);
    }
  }

  inline bool receive(zaf::MessageHandlers& handlers, bool non_blocking) {
    return forwarder->receive_once(handlers, non_blocking);
  }

private:
  std::vector<zaf::Actor> executors;
  zaf::ScopedActor<zaf::ActorBehaviorX> forwarder;
  KeyBy key_by;
  std::hash<KeyType> hasher{};
};

template<typename I>
class OnDemandAssign {
public:
  inline void initialize(zaf::ActorGroup& group, std::vector<zaf::Actor>& executors) {
    forwarder = group.create_scoped_actor<zaf::ActorBehaviorX>();
    dispatcher = group.spawn<Dispatcher>(executors, forwarder->get_self_actor());
  }

  template<typename U>
  inline void dispatch(U&& elem) {
    forwarder->send(dispatcher, codes::Data, std::forward<U>(elem));
  }

  inline void terminate() {
    forwarder->send(dispatcher, codes::Termination);
  }

  inline void clear() {
    forwarder->send(dispatcher, codes::Clear);
    forwarder = nullptr;
  }

  inline bool receive(zaf::MessageHandlers& handlers, bool non_blocking) {
    return forwarder->receive_once(handlers, non_blocking);
  }

private:
  class Dispatcher : public zaf::ActorBehaviorX {
  public:
    Dispatcher(const std::vector<zaf::Actor>& executors, zaf::Actor res_collector):
      res_collector(res_collector),
      quotas(executors.size()),
      executors(executors) {
    }

    void start() override {
      for (size_t i = 0, n = executors.size(); i < n; i++) {
        this->send(executors[i], codes::Quota, i);
        this->send(executors[i], codes::Downstream, res_collector);
      }
      for (size_t i = 0, n = executors.size(); i < n; i++) {
        quotas[i] = i;
      }
    }

    zaf::MessageHandlers behavior() override {
      return {
        codes::Data - [=](I&& elem) {
          if (quotas.empty()) {
            buffered_elems.emplace_back(std::forward<I>(elem));
          } else {
            auto w = quotas.back();
            this->send(executors[w], codes::DataWithQuota, std::forward<I>(elem), w);
            quotas.pop_back();
          }
        },
        codes::Quota - [=](size_t w) {
          if (buffered_elems.empty()) {
            quotas.push_back(w);
          } else {
            this->send(executors[w], codes::DataWithQuota, std::move(buffered_elems.front()), w);
            buffered_elems.pop_front();
            if (buffered_elems.empty() && flag_termination) {
              terminate();
            }
          }
        },
        codes::Termination - [=]() {
          if (buffered_elems.empty()) {
            terminate();
          } else {
            flag_termination = true;
          }
        },
        codes::Clear - [=]() {
          terminate();
        }
      };
    }

    void terminate() {
      for (auto& w : executors) {
        this->send(w, codes::Termination);
      }
      this->deactivate();
    }

    zaf::Actor res_collector;
    bool flag_termination = false;
    std::deque<I> buffered_elems;
    std::vector<size_t> quotas;
    std::vector<zaf::Actor> executors;
  };

  zaf::ScopedActor<zaf::ActorBehaviorX> forwarder;
  zaf::Actor dispatcher;
};
} // namespace details

struct RandomAssign {
  template<typename I>
  using type = details::RandomAssign<I>;

  template<typename I>
  inline details::RandomAssign<I> create() const { return {}; }
};

template<typename KeyBy>
struct Partition {
  KeyBy key_by;

  Partition(const KeyBy& key_by): key_by(key_by) {}

  template<typename I>
  using type = details::Partition<I, KeyBy>;

  template<typename I>
  inline details::Partition<I, KeyBy> create() const { return {key_by}; }
};

struct OnDemandAssign {
  template<typename I>
  using type = details::OnDemandAssign<I>;

  template<typename I>
  inline details::OnDemandAssign<I> create() const { return {}; }
};
} // namespace shuffle
} // namespace coll
#endif
