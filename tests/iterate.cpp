#include "coll/coll.hpp"
#include "gtest/gtest.h"

GTEST_TEST(Iterate, Basic) {
  std::vector<int> x{1, 2, 3, 4, 5, 6};
  int c = 0;
  coll::iterate(x)
    | coll::foreach([&](auto&& i) {
        EXPECT_EQ(++c, i);
      });
  EXPECT_EQ(c, (int) x.size());
}

GTEST_TEST(Iterate, String) {
  auto str = std::string("123456789");
  int c = 0;
  coll::iterate(str)
    | coll::foreach([&](auto&& i) {
        EXPECT_EQ(++c + '0', i);
      });
  EXPECT_EQ(c, (int) str.size());
}

GTEST_TEST(Iterate, BoundedArray) {
  int a[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  int c = 0;
  coll::iterate(a)
    | coll::foreach([&](auto&& i) {
        EXPECT_EQ(++c, i);
      });
  EXPECT_EQ(c, 10);
}

GTEST_TEST(Iterate, PostIterate) {
  auto vec = std::vector<int>{1, 2, 3, 4, 5, 6, 7, 8, 9};
  int c = 0;
  vec | coll::iterate()
      | coll::foreach([&](auto&& i) {
        EXPECT_EQ(++c, i);
      });
  EXPECT_EQ(c, (int) vec.size());
}

GTEST_TEST(Iterate, Optional) {
  std::optional<int> r;
  coll::range(10)
    | coll::filter(anony_cc(_ % 2 == 1))
    | coll::head()
    | coll::iterate()
    | coll::foreach(anonyr_av(r = _));
  EXPECT_EQ(r, 1);

  r = std::nullopt;
  coll::range(10)
    | coll::filter(anony_cc(false))
    | coll::head()
    | coll::iterate()
    | coll::foreach(anonyr_av(r = _));
  EXPECT_FALSE(bool(r));
}
