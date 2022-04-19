#pragma once

#include <optional>

namespace std {
template<typename T>
class optional<T&> {
private:
  T* ref = nullptr;

public:
  optional() = default;
  optional(std::nullopt_t) {};
  optional(const optional<T&>&) = default;

  optional(optional<T&>&& other) noexcept:
    ref(other.ref) {
    other.ref = nullptr;
  }

  template<typename U>
  optional(const optional<U&>& other):
    ref(&other.value()) {
  }

  template<typename U>
  optional(optional<U&>&& other):
    ref(&other.value()) {
    other = std::nullopt;
  }

  template<typename U,
    typename std::enable_if_t<std::is_convertible<U*, T*>::value>* = nullptr>
  optional(U& v): ref(&v) {}

  inline optional<T&>& operator=(std::nullopt_t) noexcept {
    ref = nullptr;
    return *this;
  }

  template<typename U = T>
  inline optional<T&>& operator=(const optional<U&>& other) {
    ref = other.ref;
    return *this;
  }

  template<typename U = T>
  inline optional<T&>& operator=(optional<U&>&& other) {
    ref = other.ref;
    other.ref = nullptr;
    return *this;
  }

  template<typename U = T>
  inline optional<T&>& operator=(U& other_value) {
    ref = &other_value;
    return *this;
  }

  inline operator bool() const noexcept { return bool(ref); }
  inline bool has_value() const noexcept { return bool(ref); }

  inline T& value() {
    if (ref == nullptr) {
      throw std::runtime_error("Attempt to use nullptr reference.");
    }
    return *ref;
  }

  inline const T& value() const {
    if (ref == nullptr) {
      throw std::runtime_error("Attempt to use nullptr reference.");
    }
    return *ref;
  }

  template<typename U>
  inline auto value_or(U&& default_val) {
    return bool(ref)
      ? *ref
      : std::forward<U>(default_val);
  }

  inline T& operator* () {
    if (ref == nullptr) {
      throw std::runtime_error("Attempt to use nullptr reference.");
    }
    return *ref;
  }

  inline const T& operator* () const {
    if (ref == nullptr) {
      throw std::runtime_error("Attempt to use nullptr reference.");
    }
    return *ref;
  }

  inline T* operator-> () {
    if (ref == nullptr) {
      throw std::runtime_error("Attempt to use nullptr reference.");
    }
    return ref;
  }

  inline const T* operator-> () const {
    if (ref == nullptr) {
      throw std::runtime_error("Attempt to use nullptr reference.");
    }
    return ref;
  }

  inline void swap(optional<T&>& other) { std::swap(ref, other.ref); }

  inline void reset() { ref = nullptr; }

  template<typename U>
  inline T& emplace(U& other_value) {
    if (!ref) {
      ref = &other_value;
    }
    return *ref;
  }
};

template<typename T>
inline bool operator<(const optional<T&>& a, const optional<T&>& b) {
  return bool(a)
    ? bool(b)
      ? *a < *b // both valid
      : false   // a is not empty > b is empty
    : !bool(b); // b is empty => equal; b is not empty => a < b;
}

template<typename T>
inline bool operator==(const optional<T&>& a, const optional<T&>& b) {
  return bool(a)
    ? bool(b) && *a == *b
    : !bool(b);
}

template<typename T>
struct hash<optional<T&>> {
  inline size_t operator()(const optional<T&>& r) const {
    return bool(r)
      ? 0
      : std::hash<T>{}(*r);
  }
};
} // namespace std

namespace coll {
template<typename T>
using Reference = std::optional<T&>;
} // namespace coll
