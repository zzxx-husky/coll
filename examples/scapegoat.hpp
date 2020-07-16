#pragma once

#include <iostream>

struct ScapegoatCounter {
  static int num_scapegoat_copy;
  static int num_scapegoat_move;

  static void clear() {
    num_scapegoat_copy = num_scapegoat_move = 0;
  }

  static void status() {
    std::cout << "num_scapegoat_copy: " << num_scapegoat_copy
      << ", num_scapegoat_move: " << num_scapegoat_move << std::endl;
  }
};

int ScapegoatCounter::num_scapegoat_copy = 0;
int ScapegoatCounter::num_scapegoat_move = 0;

struct Scapegoat {
  int val = 0;

  Scapegoat() = default;

  Scapegoat(int val): val(val) {}

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
    return {val + a};
  }

  Scapegoat operator+ (const Scapegoat& other) {
    return {val + other.val};
  }

  template<typename Other>
  friend Other operator+ (Other& o, const Scapegoat& s) {
    return o + s.val;
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
