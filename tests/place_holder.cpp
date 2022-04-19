#include "coll/coll.hpp"
#include "gtest/gtest.h"

GTEST_TEST(PlaceHolder, PostIterateExecutable) {
  auto x = coll::place_holder<int>() | coll::to<std::vector>();
  int c = 0;
  auto y = x | coll::iterate()
    | coll::foreach([&](auto&& i) {
      EXPECT_EQ(++c, i);
    });
  int a[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  y.start();
  for (int i = 0; i < 10; i++) {
    y.run(a[i]);
  }
  y.end();
  EXPECT_EQ(c, 10);
}

GTEST_TEST(PlaceHolder, PostPlaceHolder) {
  auto x = coll::place_holder<int>()
    | coll::to<std::vector>();
  auto y = coll::place_holder<int>()
    | coll::map(anony_cc(_ + 1));
  auto z = coll::range(10) | y | x;
  EXPECT_EQ((int) z.size(), 10);
  for (int i = 0; i < 10; i++) {
    EXPECT_EQ(i + 1, z[i]);
  }
}

GTEST_TEST(PlaceHolder, PostPlaceHolder2) {
  auto x = coll::place_holder<int>()
    | coll::to<std::vector>();
  auto y = coll::place_holder<int>()
    | coll::map(anony_cc(_ + 1));
  auto w = coll::place_holder<int>()
    | coll::map(anony_cc(_));
  auto z = coll::range(10) | w | y | x;
  EXPECT_EQ((int) z.size(), 10);
  for (int i = 0; i < 10; i++) {
    EXPECT_EQ(i + 1, z[i]);
  }
}

GTEST_TEST(PlaceHolder, Concat) {
  std::vector<int> v;

  auto x = coll::place_holder<int>()
    | coll::map(anony_cc(_ + 1));
  auto y = coll::place_holder<int>()
    | coll::map(anony_cc(_ * 2));
  auto z = x | coll::concat(y)
    | coll::foreach([&](auto i) {
        v.push_back(i);
      });

  z.start();

  z.run(coll::Left::value, 1);
  EXPECT_EQ((int) v.size(), 1);
  EXPECT_EQ((int) v.back(), 1 + 1);

  z.run(coll::Right::value, 2);
  EXPECT_EQ((int) v.size(), 2);
  EXPECT_EQ(v.back(), 2 * 2);

  z.run(coll::Left::value, 3);
  EXPECT_EQ((int) v.size(), 3);
  EXPECT_EQ(v.back(), 3 + 1);

  z.run(coll::Right::value, 4);
  EXPECT_EQ((int) v.size(), 4);
  EXPECT_EQ(v.back(), 4 * 2);

  z.end();
}

GTEST_TEST(PlaceHolder, Concat2) {
  std::vector<int> v;

  auto x = coll::place_holder<int>()
    | coll::map(anony_cc(_ + 1));
  auto y = coll::place_holder<int>()
    | coll::map(anony_cc(_ * 2));
  auto z = x | coll::concat(y);

  auto a = coll::place_holder<int>()
    | coll::map(anony_cc(_ - 3));
  auto b = coll::place_holder<int>()
    | coll::map(anony_cc(_ / 4));
  auto c = a | coll::concat(b);

  auto w = z | concat(c)
    | coll::foreach([&](auto i) {
        v.push_back(i);
      });

  w.start();

  w.run(coll::Left::value, coll::Left::value, 1);
  EXPECT_EQ((int) v.size(), 1);
  EXPECT_EQ(v.back(), 1 + 1);

  w.run(coll::Left::value, coll::Right::value, 2);
  EXPECT_EQ((int) v.size(), 2);
  EXPECT_EQ(v.back(), 2 * 2);

  w.run(coll::Right::value, coll::Left::value, 3);
  EXPECT_EQ((int) v.size(), 3);
  EXPECT_EQ(v.back(), 3 - 3);

  w.run(coll::Right::value, coll::Right::value, 4);
  EXPECT_EQ((int) v.size(), 4);
  EXPECT_EQ(v.back(), 4 / 4);

  w.end();
}
