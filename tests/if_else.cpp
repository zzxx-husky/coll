#include <vector>

#include "coll/coll.hpp"
#include "gtest/gtest.h"

GTEST_TEST(IfElse, Basic) {
  std::vector<int> even, odd;
  coll::range(100)
    | coll::if_else(
        anony_cc(_ % 2 == 0),
        anonyr_cc(_ | coll::to(even)),
        anonyr_cc(_ | coll::to(odd))
      );

  EXPECT_EQ(even.size(), 100 / 2);
  EXPECT_EQ(odd.size(), 100 / 2);
  EXPECT_TRUE(coll::iterate(even) | coll::all(anony_cc(_ % 2 == 0)));
  EXPECT_TRUE(coll::iterate(odd) | coll::all(anony_cc(_ % 2 == 1)));
}

GTEST_TEST(IfElse, NestedIfElse) {
  std::vector<int> a, b, c, d;
  coll::range(100)
    | coll::if_else(
        anony_cc(_ % 2 == 0),
        anonyr_cc(
          _ | coll::if_else(
                anony_cc((_ >> 1) % 2 == 0),
                anonyr_cc(_ | coll::to(a)),
                anonyr_cc(_ | coll::to(b))
              )),
        anonyr_cc(
          _ | coll::if_else(
                anony_cc((_ >> 1) % 2 == 0),
                anonyr_cc(_ | coll::to(c)),
                anonyr_cc(_ | coll::to(d))
              ))
      );

  EXPECT_EQ(a.size(), 100 / 4);
  EXPECT_EQ(b.size(), 100 / 4);
  EXPECT_EQ(c.size(), 100 / 4);
  EXPECT_EQ(d.size(), 100 / 4);
  EXPECT_TRUE(coll::iterate(a) | coll::all(anony_cc(_ % 4 == 0)));
  EXPECT_TRUE(coll::iterate(b) | coll::all(anony_cc(_ % 4 == 2)));
  EXPECT_TRUE(coll::iterate(c) | coll::all(anony_cc(_ % 4 == 1)));
  EXPECT_TRUE(coll::iterate(d) | coll::all(anony_cc(_ % 4 == 3)));
}

GTEST_TEST(IfElse, WithConcat) {
  std::vector<int> even, odd;
  coll::range(100)
    | coll::if_else(
        anony_cc(_ % 2 == 0),
        anonyr_cc(_ | coll::concat(coll::elements(100)) | coll::to(even)),
        anonyr_cc(_ | coll::concat(coll::elements(101)) | coll::to(odd))
      );

  EXPECT_EQ(even.size(), 100 / 2 + 1);
  EXPECT_EQ(odd.size(), 100 / 2 + 1);
  EXPECT_TRUE(coll::iterate(even) | coll::all(anony_cc(_ % 2 == 0)));
  EXPECT_TRUE(coll::iterate(odd) | coll::all(anony_cc(_ % 2 == 1)));
}
