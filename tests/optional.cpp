#include "coll/coll.hpp"
#include "gtest/gtest.h"

GTEST_TEST(Optional, Map) {
  std::optional<int> a{1};
  
  auto b = a | coll::map(anony_cc(_ * 2));
  EXPECT_EQ(b, 2);

  a = std::nullopt;
  b = a | coll::map(anony_cc(_ * 2));
  EXPECT_FALSE(bool(b));
}

GTEST_TEST(Optional, Filter) {
  std::optional<int> a{1};
  
  auto b = a | coll::filter(anony_cc(_ % 2 == 1));
  EXPECT_EQ(b, a);

  b = a | coll::filter(anony_cc(_ % 2 == 0));
  EXPECT_FALSE(bool(b));

  a = std::nullopt;
  b = a | coll::filter(anony_cc(_ % 2 == 1));
  EXPECT_FALSE(bool(b));
}

GTEST_TEST(Optional, Flatmap) {
  std::optional<int> a{2};

  auto b = a
    | coll::flatmap(anony_cc(std::vector<int>(2, 10)))
    | coll::to_vector();
  EXPECT_EQ(b, (std::vector<int>{10, 10}));

  a = std::nullopt;
  b = a
    | coll::flatmap(anony_cc(std::vector<int>(2, 10)))
    | coll::to_vector();
  EXPECT_TRUE(b.empty());

  std::vector<int> x{1,2,3,4};
  a = {1};
  auto s = a
    | coll::flatmap(anonyr_cr(x))
    | coll::sum()
    | coll::unwrap();
  EXPECT_EQ(s, 10);
}

GTEST_TEST(Optional, MapFilterFlatmap) {
  std::optional<int> a{3};
  auto b = a
    | coll::map(anony_cc(_ - 1))
    | coll::filter(anony_cc(_ % 2 == 0))
    | coll::flatmap(anony_cc(std::vector<int>(_, 11)))
    | coll::to_vector();

  EXPECT_EQ(b, (std::vector<int>{11, 11}));
}

GTEST_TEST(Optional, Foreach) {
  int v = 0;
  std::optional<int> a{2};
  a | coll::foreach(anonyr_cv(v = _));
  EXPECT_EQ(v, 2);

  v = 0;
  a = std::nullopt;
  a | coll::foreach(anonyr_cv(v = _));
  EXPECT_EQ(v, 0);
}

GTEST_TEST(Optional, Inspect) {
  int v = 0;
  std::optional<int> a{2};
  a | coll::inspect(anonyr_cv(v++))
    | coll::filter(anony_cc(_ != 2))
    | coll::inspect(anonyr_cv(v = -v))
    | coll::act();

  EXPECT_EQ(v, 1);
}
