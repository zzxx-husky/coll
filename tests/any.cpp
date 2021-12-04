#include "coll/coll.hpp"
#include "gtest/gtest.h"

GTEST_TEST(Any, Basic) {
  auto any_odd = coll::range(100)
    | coll::concat(coll::elements(1))
    | coll::any(anony_cc(_ % 2 == 1));

  EXPECT_TRUE(any_odd);
}

GTEST_TEST(Any, Basic2) {
  auto any_even = coll::range(100)
    | coll::filter(anony_cc(_ % 2 == 1))
    | coll::any(anony_cc(_ % 2 == 0));

  EXPECT_FALSE(any_even);
}

GTEST_TEST(Any, Empty) {
  auto any_even = coll::range(0)
    | coll::filter(anony_cc(_ % 2 == 0))
    | coll::any(anony_cc(_ % 2 == 0));

  EXPECT_FALSE(any_even);
}
