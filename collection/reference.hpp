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

  template<typename U = T>
  optional(optional<T&>&& other) noexcept:
    ref(other.ref) {
    other.ref = nullptr;
  }

  template<typename U = T>
  optional(optional<U&>&& other):
    ref(other.ref) {
  }

  template<typename U,
    std::enable_if_t<std::is_convertible<U*, T*>::value>* = nullptr>
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

  inline T& value() { return *ref; }
  inline const T& value() const { return *ref; }

  template<typename U>
  inline auto value_or(U&& default_val) {
    return bool(ref)
      ? *ref
      : std::forward<U>(default_val);
  }

  inline T& operator* () { return *ref; }
  inline const T& operator* () const { return *ref; }

  inline T* operator-> () { return ref; }
  inline const T* operator-> () const { return ref; }

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
  return *a < *b;
}

template<typename T>
inline bool operator==(const optional<T&>& a, const optional<T&>& b) {
  return *a == *b;
}

template<typename T>
struct hash<optional<T&>> {
  inline size_t operator()(const optional<T&>& r) const {
    return std::hash<T>{}(*r);
  }
};
} // namespace std

namespace coll {
template<typename T>
using Reference = std::optional<T&>;
} // namespace coll
