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
  vec | coll::iterate()
      | coll::foreach([&](auto&& i) {
        EXPECT_EQ(++c, i);
      });
  EXPECT_EQ(c, vec.size());
}

GTEST_TEST(Iterate, PostIterateExecutable) {
  auto x = coll::place_holder<int>() | coll::to<std::vector>();
  int c = 0;
  auto y = x | coll::iterate()
    | coll::foreach([&](auto&& i) {
      EXPECT_EQ(++c, i);
    });
  int a[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  y.start();
  for (int i = 0; i < 10; i++) {
    y.process(a[i]);
  }
  y.end();
  EXPECT_EQ(c, 10);
}

GTEST_TEST(Iterate, PostPlaceHolder) {
  auto x = coll::place_holder<int>()
    | coll::to<std::vector>();
  auto y = coll::place_holder<int>()
    | coll::map(anony_cc(_ + 1));
  auto z = coll::range(10) | y | x;
  EXPECT_EQ(z.size(), 10);
  for (int i = 0; i < 10; i++) {
    EXPECT_EQ(i + 1, z[i]);
  }
}

GTEST_TEST(Iterate, PostPlaceHolder2) {
  auto x = coll::place_holder<int>()
    | coll::to<std::vector>();
  auto y = coll::place_holder<int>()
    | coll::map(anony_cc(_ + 1));
  auto w = coll::place_holder<int>()
    | coll::map(anony_cc(_));
  auto z = coll::range(10) | w | y | x;
  EXPECT_EQ(z.size(), 10);
  for (int i = 0; i < 10; i++) {
    EXPECT_EQ(i + 1, z[i]);
  }
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
