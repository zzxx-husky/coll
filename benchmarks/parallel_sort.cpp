#include <algorithm>
#include <chrono>
#include <queue>
#include <vector>

#include "coll/coll.hpp"
#include "coll/parallel_coll.hpp"
#include "coll/topk.hpp"

int main() {
  const int N = 50000000;
  auto ints = coll::range(N)
    | coll::map(anony_cc(rand()))
    | coll::to_vector(N);

  auto duration = [](auto exec) {
    auto start = std::chrono::steady_clock::now();
    exec();
    auto end = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
  };

  auto ints1 = ints;
  auto std_sort_time = duration([&]() {
    std::sort(ints1.begin(), ints1.end());
  });
  std::cout << "std::sort duration: " << std_sort_time << " ms." << std::endl;

  auto ints2 = ints;
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
  std::cout << "ints is" << (ints1 == ints2 ? " " : " not ") << "the same as ints2." << std::endl;
  ints2.clear();

  auto ints3 = ints;
  auto terasort_time = duration([&]() {
    const int T = 4;      // num of threads
    const int M = T * 2;  // num of mapper partitions
    const int R = T;      // num of reducer partitions
    auto seeds = coll::range(R * 10)
      | coll::map(anonyr_cc(ints2[abs(rand()) % N]))
      | coll::sort()
      | coll::window(1, 10)       // because R * 10
      | coll::map(anony_rc(_[0])) // the first and the only element in the window
      | coll::tail()              // skip the last window
      | coll::to<std::vector>();  // size of vector: R

    zaf::ActorEngine actor_engine{coll::parallel_utils::actor_system, T};

    auto sorted_ints3 = coll::range(M)
      | coll::parallel(M, [&](auto _, auto in) {
          return in | coll::flatmap([&](size_t pid) {
            auto par_begin = ints3.size() / M * pid + std::min(pid, ints3.size() % M);
            auto par_end = par_begin + ints3.size() / M + (pid < ints3.size() % M);
            return coll::iterate(ints3.begin() + par_begin, ints3.begin() + par_end)
              | coll::groupby(anonyr_cc(std::lower_bound(seeds.begin(), seeds.end(), _) - seeds.begin()))
                  .to_vector();
          });
        })
        .execute_by(actor_engine)
      | coll::parallel_partition(R, [&](auto _, auto in) {
          return in
            | coll::flatmap(anony_rr(_.second))
            | coll::sort()
            | coll::to<std::vector>();
        })
        .key_by(anony_ar(_.first))
        .execute_by(actor_engine)
      | coll::aggregate(anony_ac(std::vector<std::vector<int>>(R)),
          [](auto& vs, auto&& v) {
            vs[v.first] = std::move(v.second);
          })
      | coll::iterate()
      | coll::aggregate([=](auto type) {
          std::vector<int> vec;
          vec.reserve(N);
          return vec;
        }, [](auto& vec, auto&& x) {
          vec.insert(vec.end(), x.begin(), x.end());
        });
    sorted_ints3.swap(ints3);
  });
  std::cout << "terasort duration: " << terasort_time << " ms." << std::endl;
  std::cout << "ints is" << (ints1 == ints3 ? " " : " not ") << "the same as ints3." << std::endl;
  ints3.clear();
}
