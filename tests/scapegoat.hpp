#pragma once

#include <iostream>

struct ScapegoatCounter {
  inline static int num_scapegoat_copy;
  inline static int num_scapegoat_move;

  static void clear() {
    num_scapegoat_copy = num_scapegoat_move = 0;
  }

  static void status() {
    std::cout << "num_scapegoat_copy: " << num_scapegoat_copy
      << ", num_scapegoat_move: " << num_scapegoat_move << std::endl;
  }
};

struct Scapegoat {
  int val = 0;

  Scapegoat() = default;

  explicit Scapegoat(int val): val(val) {}

  Scapegoat(const Scapegoat& o) {
    val = o.val;
    ++ScapegoatCounter::num_scapegoat_copy;
  }

  Scapegoat(Scapegoat&& o) {
    val = o.val;
    ++ScapegoatCounter::num_scapegoat_move;
  }

  Scapegoat& operator+= (int a) {
    val += a;
    return *this;
  }

  Scapegoat& operator+= (const Scapegoat& other) {
    val += other.val;
    return *this;
  }

  Scapegoat operator+ (int a) {
    return Scapegoat{val + a};
  }

  Scapegoat operator+ (const Scapegoat& other) {
    return Scapegoat{val + other.val};
  }

  template<typename Other>
  friend Other operator+ (Other& o, const Scapegoat& s) {
    return o + s.val;
  }

  Scapegoat& operator*= (int a) {
    val *= a;
    return *this;
  }

  Scapegoat& operator*= (const Scapegoat& other) {
    val *= other.val;
    return *this;
  }

  Scapegoat operator* (int a) {
    return Scapegoat{val * a};
  }

  Scapegoat operator* (const Scapegoat& other) {
    return Scapegoat{val * other.val};
  }

  template<typename Other>
  friend Other operator* (Other& o, const Scapegoat& s) {
    return o * s.val;
  }

  auto& operator=(const Scapegoat& o) {
    val = o.val;
    ++ScapegoatCounter::num_scapegoat_copy;
    return *this;
  }

  auto& operator=(Scapegoat&& o) {
    val = o.val;
    ++ScapegoatCounter::num_scapegoat_move;
    return *this;
  }

  friend inline bool operator<(const Scapegoat& a, const Scapegoat& b) {
    return a.val < b.val;
  }

  friend std::ostream& operator<<(std::ostream& out, const Scapegoat& goat) {
    out << "Scapegoat{" << goat.val << "}";
    return out;
  }

  friend inline bool operator==(const Scapegoat& a, const Scapegoat& b) {
    return a.val == b.val;
  }

  friend inline bool operator!=(const Scapegoat& a, const Scapegoat& b) {
    return a.val != b.val;
  }
};

namespace std {
template<>
struct hash<Scapegoat> {
  size_t operator()(const Scapegoat& a) const {
    return a.val;
  }
};
};
