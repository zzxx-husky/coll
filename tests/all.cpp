#include "coll/coll.hpp"
#include "gtest/gtest.h"

GTEST_TEST(All, Basic) {
  auto all_even = coll::range(100)
    | coll::filter(anony_cc(_ % 2 == 0))
    | coll::all(anony_cc(_ % 2 == 0));

  EXPECT_TRUE(all_even);
}

GTEST_TEST(All, Empty) {
  auto all_even = coll::range(0)
    | coll::filter(anony_cc(_ % 2 == 1))
    | coll::all(anony_cc(_ % 2 == 0));

  EXPECT_TRUE(all_even);
}
