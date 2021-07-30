#include "coll/coll.hpp"
#include "gtest/gtest.h"

GTEST_TEST(Iterate, Basic) {
  std::vector<int> x{1, 2, 3, 4, 5, 6};
  int c = 0;
  coll::iterate(x)
    | coll::foreach([&](auto&& i) {
        EXPECT_EQ(++c, i);
      });
  EXPECT_EQ(c, x.size());
}

GTEST_TEST(Iterate, String) {
  auto str = std::string("123456789");
  int c = 0;
  coll::iterate(str)
    | coll::foreach([&](auto&& i) {
        EXPECT_EQ(++c + '0', i);
      });
  EXPECT_EQ(c, str.size());
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
  vec | coll::PostIterate()
      | coll::foreach([&](auto&& i) {
        EXPECT_EQ(++c, i);
      });
  EXPECT_EQ(c, vec.size());
}

GTEST_TEST(Iterate, PostIterateExecutable) {
  auto x = coll::place_holder<int>() | coll::to<std::vector>();
  int c = 0;
  auto y = x | coll::PostIterate()
    | coll::foreach([&](auto&& i) {
      EXPECT_EQ(++c, i);
    });
  int a[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  for (int i = 0; i < 10; i++) {
    y.process(a[i]);
  }
  y.end();
  EXPECT_EQ(c, 10);
}
