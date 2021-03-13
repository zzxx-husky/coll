#include "collection/coll.hpp"

template<typename T>
std::ostream& operator<<(std::ostream& out, const std::optional<T>& o) {
  if (bool(o)) {
    out << "Some(" << *o << ")";
  } else {
    out << "None";
  }
  return out;
}

int main() {
  int n = 4;
  int s = (0 + 100 - 1) * 100 / 2;

  std::cout << "Expected sum: " << s << std::endl;

  s = *(coll::range(100)
    | coll::parallel(n, [](size_t pid, auto in) {
        return in | coll::sum();
      })
    | coll::inspect([](auto& i2e) {
        std::cout << "Partition " << i2e.first << " sum: " << i2e.second << std::endl;
      })
    | coll::map(anony_ac(_.second.value_or(0)))
    | coll::sum());

  std::cout << "(Sum of Partition) Total sum using " << n << " threads: " << s << std::endl;

  s = *(coll::range(100)
    // | coll::inspect([](auto& e) {
    //     std::cout << "Generated " << e << std::endl;
    //   })
    | coll::parallel(n, [](size_t pid, auto in) {
        return in | coll::head();
      })
    | coll::inspect([](auto& i2e) {
        std::cout << "Partition " << i2e.first << " head: " << i2e.second << std::endl;
      })
    | coll::map(anony_ac(_.second.value_or(0)))
    | coll::sum());

  std::cout << "(Sum of Partition Head) Total sum using " << n << " threads: " << s << std::endl;

  s = *(coll::range(100)
    | coll::parallel(n, [](size_t pid, auto in) {
        return in | coll::map(anony_cc(_ * 2));
      })
    | coll::sum());

  std::cout << "(Sum of Twice Partition) Total sum using " << n << " threads: " << s << std::endl;

  s = *(coll::range(3)
    | coll::parallel(n, [](size_t pid, auto in) {
        return in | coll::map(anony_cc(_ * 2));
      })
    | coll::sum());

  std::cout << "(Sum of Twice Partition with few ints) Total sum using " << n << " threads: " << s << std::endl;

  coll::range(8)
    | coll::parallel(n, [](size_t pid, auto in) {
        return in
          | coll::println().format([pid](auto& out, auto&& e) {
              out << "Thread " << pid << " gets " << e;
            });
      })
    | coll::println().format([](auto& out, auto&& pid) {
        out << "Thread " << pid << " now ends.";
      });

  std::cout << "Parallel Partition with sink operator having no result." << std::endl;

  s = *(coll::range(100)
    | coll::partition([](int key, auto in) {
        return in | coll::sum();
      })
      .by(anony_cc(_ % 4))
      .parallel(4)
    | coll::inspect([](auto& i2e) {
        std::cout << "Partition " << i2e.first << " sum: " << i2e.second << std::endl;
      })
    | coll::map(anony_ac(*_.second))
    | coll::sum());

  std::cout << "Total sum: " << s << std::endl;
}
