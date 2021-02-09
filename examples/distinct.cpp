#include <cstdlib>
#include <ctime>
#include <iostream>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <vector>

#include "collection/coll.hpp"

#include "scapegoat.hpp"

int main() {
  srand(time(0));

  auto size = 1000;
  auto v = coll::range(size)
    | coll::map(anonyr_cc(rand() % size))
    | coll::to<std::vector>();

  auto num_distinct_default = coll::iterate(v)
    | coll::distinct()
    | coll::count();

  auto num_distinct_unordred_set = coll::iterate(v)
    | coll::distinct<std::unordered_set>()
    | coll::count();

  auto num_distinct_set = coll::iterate(v)
    | coll::distinct<std::set>()
    | coll::count();

  auto num_distinct_vector = coll::iterate(v)
    | coll::distinct<std::vector>([](auto& set, auto&& e) -> bool {
        if (std::find(set.begin(), set.end(), e) == set.end()) {
          set.push_back(e);
          return true;
        }
        return false;
      })
    | coll::count();

  auto actual_distinct = std::unordered_set<int>(v.begin(), v.end()).size();

  std::cout << "Num of integers: " << size << std::endl
   << "Num of distinct integers (default): " << num_distinct_default << std::endl
   << "Num of distinct integers (unordered_set): " << num_distinct_unordred_set << std::endl
   << "Num of distinct integers (set): " << num_distinct_set << std::endl
   << "Num of distinct integers (vector): " << num_distinct_vector << std::endl
   << "Actual num of distinct integers: " << actual_distinct << std::endl;

  std::vector<Scapegoat> s(1000);
  ScapegoatCounter::clear();
  std::cout << "Before distinct (scapegoat) ";
  ScapegoatCounter::status();
  auto num_distinct_scapegoat = coll::iterate(s)
    | coll::distinct<std::vector>([](auto& set, auto&& e) -> bool {
        set.push_back(e);
        return true;
      })
    | coll::count();
  std::cout << "After distinct (scapegoat) ";
  ScapegoatCounter::status();
  std::cout << "Num of distinct (scapegoat): " << num_distinct_scapegoat << ". Expect: " << s.size() << std::endl;

  ScapegoatCounter::clear();
  std::cout << "Before distinct (scapegoat + ref) ";
  ScapegoatCounter::status();
  num_distinct_scapegoat = coll::iterate(s)
    | coll::distinct<std::vector>([](auto& set, auto&& e) -> bool {
        set.push_back(e);
        return true;
      })
      .cache_by_ref()
    | coll::count();
  std::cout << "After distinct (scapegoat) ";
  ScapegoatCounter::status();
  std::cout << "Num of distinct (scapegoat + ref): " << num_distinct_scapegoat << ". Expect: " << s.size() << std::endl;
}
