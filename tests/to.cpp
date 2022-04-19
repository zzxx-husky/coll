#include <deque>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <stack>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>

#include "coll/coll.hpp"
#include "gtest/gtest.h"

template <template<typename ...> class C, bool check_equivalence = true>
void new_container(const std::string& str) {
  auto c1 = coll::iterate(str)
    | coll::map(anony_cc(char(std::toupper(_))))
    | coll::to<C>();
  C<char> c2;
  for (auto i : str) {
    coll::container_utils::insert(c2, std::toupper(i));
  }
  EXPECT_EQ(c1.size(), c2.size());
  if constexpr (check_equivalence) {
    EXPECT_EQ(c1, c2);
  }
}

template <template<typename ...> class C, bool check_equivalence = true>
void existing_container(const std::string& str) {
  C<char> c1;
  coll::iterate(str)
    | coll::map(anony_cc(char(std::toupper(_))))
    | coll::to(c1);
  C<char> c2;
  for (auto i : str) {
    coll::container_utils::insert(c2, std::toupper(i));
  }
  EXPECT_EQ(c1.size(), c2.size());
  if constexpr (check_equivalence) {
    EXPECT_EQ(c1, c2);
  }
}

template <template<typename ...> class C>
void container_builder(const std::string& str) {
  auto c1 = coll::iterate(str)
    | coll::map(anony_cc(char(std::toupper(_))))
    | coll::to([sz = str.size()](auto type) {
        C<typename decltype(type)::type> c1;
        c1.reserve(sz);
        return c1;
      });
  C<char> c2;
  for (auto i : str) {
    coll::container_utils::insert(c2, std::toupper(i));
  }
  EXPECT_EQ(c1.size(), c2.size());
  EXPECT_EQ(c1, c2);
}

template <template<typename ...> class C>
void container_map(const std::string& str) {
  auto c1 = coll::iterate(str)
    | coll::map(anony_cc(char(std::toupper(_))))
    | coll::map(anony_cc(std::make_pair(_, _)))
    | coll::to(C<char, char>());
  C<char, char> c2;
  for (auto i : str) {
    c2.emplace(std::toupper(i), std::toupper(i));
  }
  EXPECT_EQ(c1.size(), c2.size());
  EXPECT_EQ(c1, c2);
}

GTEST_TEST(Container, Map) {
  container_map<std::map>("map");
  container_map<std::unordered_map>("unordered_map");
}

GTEST_TEST(Container, NewContainer) {
  new_container<std::deque>("deque");
  new_container<std::list>("list");
  new_container<std::priority_queue, false>("priority_queue");
  new_container<std::queue>("queue");
  new_container<std::set>("set");
  new_container<std::stack>("stack");
  new_container<std::unordered_set>("unordered_set");
  new_container<std::vector>("vector");
}

GTEST_TEST(Container, ExistingContainer) {
  existing_container<std::deque>("deque");
  existing_container<std::list>("list");
  existing_container<std::priority_queue, false>("priority_queue");
  existing_container<std::queue>("queue");
  existing_container<std::set>("set");
  existing_container<std::stack>("stack");
  existing_container<std::unordered_set>("unordered_set");
  existing_container<std::vector>("vector");
}

GTEST_TEST(Container, ContainerBuilder) {
  container_builder<std::unordered_set>("unordered_set");
  container_builder<std::vector>("vector");
}

GTEST_TEST(Container, Iterator) {
  unsigned count = 0;

  std::vector<int> a;
  coll::range(10)
    | coll::inspect(anonyr_cv(count++))
    | coll::to_iter(a.begin(), a.end());

  EXPECT_EQ(count, 0u);

  a.resize(1);
  coll::range(10)
    | coll::inspect(anonyr_cv(count++))
    | coll::to_iter(a.begin(), a.end());

  EXPECT_EQ(count, 1u);

  count = 0;
  a.resize(10);
  auto iter = coll::range(10)
    | coll::inspect(anonyr_cv(count++))
    | coll::to_iter(a.begin());

  EXPECT_EQ(count, 10u);
  EXPECT_EQ(iter, a.end());

  count = 0;
  a.resize(20);

  iter = coll::range(10)
    | coll::inspect(anonyr_cv(count++))
    | coll::to_iter(a.begin());

  EXPECT_EQ(count, 10u);
  EXPECT_EQ(iter, a.begin() + 10);
}
