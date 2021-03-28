#pragma once

#include <atomic>
#include <memory>
#include <vector>

namespace coll {
/**
 * 1. Single writer with multi readers
 * 2. Elements in the queue are shared among readers in a FCFS manner
 * 3. Designed for processing of high tput, writer (or readers) will keep busy-wait if the queue is full (or empty)
 *    For processing of low tput, std::condition_variable may be preferred.
 * 
 * T must be copyable or movable
 **/
template<typename T>
struct SWMRQueue {
  SWMRQueue() = default;

  SWMRQueue(const SWMRQueue<T>& other):
    capacity(other.capacity) {
    if (capacity != 0) {
      resize(capacity);
    }
  }

  // insert `e` into queue
  // block until the queue has available slots
  template<typename U>
  void push(U&& e) {
    auto idx = write_idx.load(std::memory_order_relaxed) & capacity_mask;
    auto& rw = status[idx];
    while (rw.load(std::memory_order_acquire) != Writable) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    elements[idx] = std::forward<U>(e);
    // relaxed here because write_idx guarantees the write
    rw.store(Readable, std::memory_order_relaxed);
    write_idx.fetch_add(1, std::memory_order_acq_rel);
  }

  inline bool is_full() const {
    auto idx = write_idx.load(std::memory_order_relaxed) & capacity_mask;
    return status[idx].load(std::memory_order_acquire) != Writable;
  }

  inline bool not_full() const {
    auto idx = write_idx.load(std::memory_order_relaxed) & capacity_mask;
    return status[idx].load(std::memory_order_acquire) == Writable;
  }

  inline bool is_ended() {
    return ended.load(std::memory_order_release); 
  }

  inline void end() {
    ended.store(true, std::memory_order_release); 
  }

  // pop an element from the queue
  // block until an available element is obtained or the queue is `end`ed
  // return `true` if an element is popped; `false` is not because the queue is ended.
  template<typename PopHandler>
  bool pop(PopHandler handler) {
    auto idx = read_idx.fetch_add(1, std::memory_order_acq_rel);
    while (write_idx.load(std::memory_order_acquire) <= idx) {
      // wait for the writer to write a new elem at `idx`
      if (ended.load(std::memory_order_acquire)) {
        if (write_idx.load(std::memory_order_acquire) <= idx) {
          return false;
        } else {
          // writer `push`s and `end`s after checking `write_idx` and before checking `end`
          break;
        }
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    idx &= capacity_mask;
    T res(std::move(elements[idx]));
    // change to writable before process it
    status[idx].store(Writable, std::memory_order_release);
    handler(res);
    return true;
  }

  // Not thread-safe. Called before Push/Pop.
  void resize(size_t sz) {
    // ensure capacity is a power of 2
    if (sz <= 2) {
      capacity = 2;
    } else {
      capacity = 0;
      for (--sz; sz; sz >>= 1, ++capacity);
      capacity = 1 << capacity;
    }
    capacity_mask = capacity - 1;
    elements.resize(capacity);
    status.reset(new std::atomic<bool>[capacity]);
    for (auto i = 0; i < capacity; i++) {
      status[i].store(Writable, std::memory_order_relaxed);
    }
  }

  constexpr static bool Writable = true;
  constexpr static bool Readable = !Writable;

  std::vector<T> elements;
  // writer syncs reads with reader by status
  // old value has been moved before a new value is written
  // a new value is written before the value gets read
  std::unique_ptr<std::atomic<bool>[]> status = nullptr;
  size_t capacity = 0, capacity_mask = 0; // init to 0, keep it to a power of 2 !
  // use atomic such that writes before end can be seen by readers when they find ended
  std::atomic<bool> ended{false};
  // writer syncs writes with reader by write_idx
  std::atomic<size_t> write_idx{0}; // the idx to write tht next elem
  // readers sync with each other by read_idx
  std::atomic<size_t> read_idx{0}; // the idx to pop the next elem
};

template<typename T>
struct MWSRQueue {
  MWSRQueue() = default;

  MWSRQueue(const MWSRQueue<T>& other):
    capacity(other.capacity) {
    if (capacity != 0) {
      resize(capacity);
    }
  }

  template<typename U>
  void push(U&& u) {
    auto idx = write_idx.fetch_add(1, std::memory_order_acq_rel);
    while (read_idx.load(std::memory_order_acquire) + capacity <= idx) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    idx &= capacity_mask;
    elements[idx] = std::forward<U>(u);
    status[idx].store(Readable, std::memory_order_release);
  }

  template<typename UGetter>
  std::optional<size_t> try_push(std::optional<size_t> idx, UGetter u) {
    if (!idx) {
      idx = write_idx.fetch_add(1, std::memory_order_acq_rel);
    }
    if (read_idx.load(std::memory_order_acquire) + capacity <= *idx) {
      return idx; // return the idx and retry in the next call
    }
    *idx &= capacity_mask;
    elements[*idx] = u();
    status[*idx].store(Readable, std::memory_order_release);
    return std::nullopt;
  }

  inline bool is_empty() const {
    auto idx = read_idx.load(std::memory_order_relaxed) & capacity_mask;
    return status[idx].load(std::memory_order_acquire) != Readable;
  }

  inline bool not_empty() const {
    auto idx = read_idx.load(std::memory_order_relaxed) & capacity_mask;
    return status[idx].load(std::memory_order_acquire) == Readable;
  }

  template<typename PopHandler>
  void pop(PopHandler handler) {
    auto idx = read_idx.load(std::memory_order_relaxed) & capacity_mask;
    auto& rw = status[idx];
    while (rw.load(std::memory_order_acquire) != Readable) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    T res(std::move(elements[idx]));
    rw.store(Writable, std::memory_order_relaxed);
    read_idx.fetch_add(1, std::memory_order_acq_rel);
    handler(res);
  }

  // Not thread-safe. Called before Push/Pop.
  void resize(size_t sz) {
    // ensure capacity is a power of 2
    if (sz <= 2) {
      capacity = 2;
    } else {
      capacity = 0;
      for (--sz; sz; sz >>= 1, ++capacity);
      capacity = 1 << capacity;
    }
    capacity_mask = capacity - 1;
    elements.resize(capacity);
    status.reset(new std::atomic<bool>[capacity]);
    for (auto i = 0; i < capacity; i++) {
      status[i].store(Writable, std::memory_order_relaxed);
    }
  }

  constexpr static bool Writable = true;
  constexpr static bool Readable = !Writable;

  std::vector<T> elements;
  std::unique_ptr<std::atomic<bool>[]> status = nullptr;
  size_t capacity = 0, capacity_mask = 0;
  std::atomic<size_t> write_idx{0};
  std::atomic<size_t> read_idx{0};
};

template<typename T>
struct SWSRQueue {
  template<typename U>
  void push(U&& u) {
    auto idx = write_idx.load(std::memory_order_relaxed) & capacity_mask;
    while (read_idx.load(std::memory_order_acquire) + capacity == idx) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    elements[idx] = std::forward<U>(u);
    write_idx.fetch_add(1, std::memory_order_acq_rel);
  }

  bool is_full() const {
    auto idx = write_idx.load(std::memory_order_relaxed) & capacity;
    return read_idx.load(std::memory_order_acquire) + capacity == idx;
  }

  bool not_full() const {
    auto idx = write_idx.load(std::memory_order_relaxed) & capacity;
    return read_idx.load(std::memory_order_acquire) + capacity > idx;
  }

  template<typename PopHandler>
  bool pop(PopHandler handler) {
    auto idx = read_idx.load(std::memory_order_relaxed) & capacity_mask;
    while (write_idx.load(std::memory_order_acquire) <= idx) {
      if (ended.load(std::memory_order_acquire)) {
        if (write_idx.load(std::memory_order_acquire) <= idx) {
          return false;
        } else {
          // writer `push`s and `end`s after checking `write_idx` and before checking `end`
          break;
        }
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    T res = std::move(elements[idx]);
    read_idx.fetch_add(1, std::memory_order_acq_rel);
    handler(res);
    return true;
  }

  inline void end() {
    ended.store(true, std::memory_order_release); 
  }

  // Not thread-safe. Called before Push/Pop.
  void resize(size_t sz) {
    // ensure capacity is a power of 2
    if (sz <= 2) {
      capacity = 2;
    } else {
      capacity = 0;
      for (--sz; sz; sz >>= 1, ++capacity);
      capacity = 1 << capacity;
    }
    capacity_mask = capacity - 1;
    elements.resize(capacity);
  }

  std::vector<T> elements;
  size_t capacity = 0, capacity_mask = 0;
  std::atomic<bool> ended{false};
  std::atomic<size_t> write_idx{0};
  std::atomic<size_t> read_idx{0};
};
} // namespace coll
