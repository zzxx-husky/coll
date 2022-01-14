#include <unordered_set>
#include <vector>

#include "coll/coll.hpp"
#include "gtest/gtest.h"

GTEST_TEST(Branch, TwoBranches) {
  std::vector<int> branch;
  std::vector<int> output;

  coll::generate(anony_c(rand() % 100)).times(10)
    | coll::branch([&](auto in) {
        return in
          | coll::sort()
          | coll::to(branch);
      })
    | coll::distinct()
    | coll::to(output);

  EXPECT_EQ(branch.size(), 10);
  EXPECT_EQ(output.size(), std::unordered_set<int>(output.begin(), output.end()).size());
  EXPECT_EQ(branch.size() - 1, 
    coll::range(branch.size() - 1)
      | coll::filter(anonyr_cc(branch[_] <= branch[_ + 1]))
      | coll::count()
  );
}

GTEST_TEST(Branch, TwoByTwoBranches) {
  std::vector<int> _1, _2, _3, _4;

  coll::generate(anony_c(rand() % 100)).times(10)
    | coll::branch([&](auto in) {
        return in
          | coll::sort()
          | coll::branch([&](auto in) {
              return in
                | coll::filter(anony_cc(_ % 2 == 0))
                | coll::to(_1);
            })
          | coll::filter(anony_cc(_ % 2 == 1))
          | coll::to(_2);
      })
    | coll::distinct()
    | coll::branch([&](auto in) {
        return in
          | coll::filter(anony_cc(_ % 2 == 0))
          | coll::to(_3);
      })
    | coll::filter(anony_cc(_ % 2 == 1))
    | coll::to(_4);

  EXPECT_EQ(_1.size() + _2.size(), 10);
  EXPECT_EQ(_1.size(),
    coll::iterate(_1)
      | coll::filter(anony_cc(_ % 2 == 0))
      | coll::count()
  );
  EXPECT_EQ(_2.size(),
    coll::iterate(_2)
      | coll::filter(anony_cc(_ % 2 == 1))
      | coll::count()
  );
  EXPECT_EQ(_3.size(), std::unordered_set<int>(_3.begin(), _3.end()).size());
  EXPECT_EQ(_3.size(),
    coll::iterate(_3)
      | coll::filter(anony_cc(_ % 2 == 0))
      | coll::count()
  );
  EXPECT_EQ(_4.size(), std::unordered_set<int>(_4.begin(), _4.end()).size());
  EXPECT_EQ(_4.size(),
    coll::iterate(_4)
      | coll::filter(anony_cc(_ % 2 == 1))
      | coll::count()
  );
}

GTEST_TEST(Branch, WithConcat) {
  std::vector<int> v;

  coll::elements(1)
    | coll::branch(anonyr_cc(
        _ | coll::concat(coll::elements(3))
          | coll::to(v)
      ))
    | coll::concat(coll::elements(2))
    | coll::to(v);

  EXPECT_EQ(v, (std::vector<int>{1, 1, 2, 3}));
}
