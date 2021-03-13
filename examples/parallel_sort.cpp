#include <algorithm>
#include <chrono>
#include <queue>
#include <vector>

#include "collection/coll.hpp"
#include "collection/topk.hpp"

int main() {
  const int N = 100000000;
  auto ints = coll::range(N)
    | coll::map(anony_cc(rand()))
    | coll::to(anonyc_cv(
        std::vector<typename decltype(_)::type> vec;
        vec.reserve(N);
        return vec;
      ));

  auto ints2 = ints;

  auto duration = [](auto exec) {
    auto start = std::chrono::steady_clock::now();
    exec();
    auto end = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
  };

  auto std_sort_time = duration([&]() {
    std::sort(ints.begin(), ints.end());
  });
  std::cout << "std::sort duration: " << std_sort_time << " ms." << std::endl;

  auto parallel_sort_time = duration([&]() {
    const int T = 4;     // number of threads
    const int P = T * 2; // number of partitions
    auto seeds = coll::range(P * 10)
      | coll::map(anonyr_cc(ints2[abs(rand()) % N]))
      | coll::sort()
      | coll::window(1, 10) // because P * 10
      | coll::map(anony_rc(_[0]))
      | coll::tail()
      | coll::to<std::vector>();
    auto sorted_ints2 = coll::range(P)
      | coll::parallel(T, [&](auto tid, auto in) {
          return in | coll::map([&](auto pid) {
            // iterate the entire ints2 and select those integers falling in the partition
            // one main cost is scanning on the entire ints2 for each partition
            return std::make_pair(pid, coll::iterate(ints2)
              | coll::filter(anonyr_cc(
                  (pid == 0 || seeds[pid - 1] <= _) &&
                  (pid == P - 1 || _ < seeds[pid])
                ))
              | coll::sort()
              | coll::to<std::vector>()
            );
          });
        })
      | coll::aggregate([=](auto type) {
          // Aggregator
          std::vector<int> vec;
          vec.reserve(N);
          return vec;
        }, [
          next_pid = 0,
          buffer = coll::topk<std::pair<int, std::vector<int>>>(P,
            [](const auto& a, const auto& b) { return b.first < a.first; }
          )
        ](auto& vec, auto&& x) mutable {
          // Aggregation method 
          if (x.first == next_pid) {
            vec.insert(vec.end(), x.second.begin(), x.second.end()); 
            for (++next_pid; !buffer.empty() && buffer.top().first == next_pid; ++next_pid) {
              auto& t = buffer.top();
              vec.insert(vec.end(), t.second.begin(), t.second.end()); 
              buffer.pop();
            }
          } else {
            buffer.push(std::move(x));
          }
        });
    sorted_ints2.swap(ints2);
  });
  std::cout << "parallel sort duration: " << parallel_sort_time << " ms." << std::endl;

  std::cout << "ints is" << (ints == ints2 ? " " : "not") << "the same as ints2." << std::endl;
}
