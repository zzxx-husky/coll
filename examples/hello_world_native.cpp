#include <array>
#include <iostream>
#include <optional>

int main() {
  bool printed_head = false;
  size_t index = 0;
  std::array<const char*, 3> in1{"let us", "say", "hello"};
  bool skipped_head = false;
  for (auto& i1 : in1) {
    if (skipped_head) {
      std::pair<size_t, const char*&> idx_i1{index++, i1};
      if (idx_i1.first % 2 == 1) {
        auto& i1_ref = idx_i1.second;
        for (auto c = i1_ref; *c; c++) {
          auto C = char(std::toupper(*c));
          if (printed_head) {
            std::cout << " ";
          } else {
            std::cout << "";
            printed_head = true;
          }
          std::cout << C;
        }
      }
    } else {
      skipped_head = true;
    }
  }
  std::array<const char*, 3> in2{"to the", "world", "loudly"};
  std::optional<const char*> prev_i2;
  for (auto& i2 : in2) {
    if (bool(prev_i2)) {
      std::pair<size_t, const char*&> idx_i2{index++, *prev_i2};
      if (idx_i2.first % 2 == 1) {
        auto& i2_ref = idx_i2.second;
        for (auto c = i2_ref; *c; c++) {
          auto C = char(std::toupper(*c));
          if (printed_head) {
            std::cout << " ";
          } else {
            std::cout << "";
            printed_head = true;
          }
          std::cout << C;
        }
      }
    }
    prev_i2 = i2;
  }
  if (!printed_head) {
    std::cout << "";
  }
  std::cout << "!\n";
}
