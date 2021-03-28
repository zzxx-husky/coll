#pragma once

#include <memory>
#include <utility>

namespace coll {
template<typename T>
class RefOrVal {
public:
  RefOrVal(T& r) {
    create_ref(true, &r);
  }

  template<typename ... U>
  RefOrVal(U&& ... u) {
    create_val(true, std::forward<U>(u)...);
  }

  RefOrVal(const RefOrVal<T>& other) {
    if (other.is_val) {
      create_val(true, other.val);
    } else {
      create_ref(true, other.ref);
    }
  }

  RefOrVal(RefOrVal<T>&& other) {
    if (other.is_val) {
      create_val(true, std::move(other.val));
    } else {
      create_ref(true, other.ref);
    }
  }

  ~RefOrVal() {
    destroy();
  }

  RefOrVal& operator=(const RefOrVal<T>& other) {
    if (other.is_val) {
      create_val(false, other.val);
    } else {
      create_ref(false, other.ref);
    } 
    return *this;
  }

  RefOrVal& operator=(RefOrVal<T>&& other) {
    if (other.is_val) {
      create_val(false, std::move(other.val));
    } else {
      create_ref(false, other.ref);
    } 
    return *this;
  }

  RefOrVal& operator=(T& r) {
    create_ref(false, &r);
    return *this;
  }

  template<typename U>
  RefOrVal& operator=(U&& u) {
    create_val(false, std::forward<U>(u));
    return *this;
  }

  template<typename ... U>
  RefOrVal& set_val(U&& ... u) {
    create_val(false, std::forward<U>(u) ...);
    return *this;
  }

  RefOrVal& set_ref(T& r) {
    create_ref(false, &r);
    return *this;
  }

  T& value() { return is_val ? val : *ref; }

  const T& value() const { return is_val ? val : *ref; }

  T* operator->() { return is_val ? &val : ref; }

  const T* operator->() const { return is_val ? &val : ref; }

  T& operator*() { return is_val ? val : *ref; }

  const T& operator*() const { return is_val ? val : *ref; }

private:
  template<typename ... U>
  void create_val(bool init, U&& ... u) {
    if (!init) {
      destroy();
    }
    is_val = true;
    new (std::addressof(val)) T(std::forward<U>(u)...);
  }

  template<typename U>
  void create_val(bool init, U&& u) {
    if (init) {
      is_val = true;
      new (std::addressof(val)) T(std::forward<U>(u));
    } else {
      if (is_val) {
        val = std::forward<U>(u);
      } else {
        destroy();
        is_val = true;
        new (std::addressof(val)) T(std::forward<U>(u));
      }
    }
  }

  void create_ref(bool init, T* r) {
    if (init) {
      is_val = false;
      new (std::addressof(ref)) T*(r);
    } else {
      if (is_val) {
        destroy();
        is_val = false;
        new (std::addressof(ref)) T*(r);
      } else {
        ref = r;
      }
    }
  }

  void destroy() {
    if (is_val) {
      val.~T();
    } else {
      using X = T*;
      ref.~X();
    }
  }

  union {
    T* ref;
    T val;
  };
  bool is_val = true;
};
} // namespace coll 
