#include "collection/coll.hpp"
#include "gtest/gtest.h"

GTEST_TEST(Partition, NoResult) {
  int num_windows = 0;
  auto vec = coll::range(100)
    | coll::partition([&](auto& key, auto in) {
        return in
          | coll::map(anonyc_cc(std::make_pair(key, _)))
          | coll::window(5)
          | coll::foreach([&](auto&&) {
              ++num_windows;
            });
      })
      .by(anony_cc(_ % 4))
    | coll::sort()
    | coll::to<std::vector>();
  EXPECT_EQ(vec, (std::vector<int>{0, 1, 2, 3}));
  EXPECT_EQ(num_windows, 100 / 5);
}

GTEST_TEST(Partition, Sum) {
  auto sums = coll::range(100)
    | coll::partition([](auto& key, auto in) {
        return in | coll::sum();
      })
      .by(anony_cc(_ % 4))
    | coll::to<std::vector>();
  EXPECT_EQ(sums.size(), 4);
  for (int i = 0; i < 4; i++) {
    EXPECT_EQ(*sums[i].second, (0 + 96 + sums[i].first * 2) * (100 / 4) / 2);
  }
}

GTEST_TEST(Partition, Pipeline) {
  auto elems = coll::range(100)
    | coll::partition([](auto& key, auto in) {
        return in
          | coll::zip_with_index()
          | coll::map(anonyc_cc(std::make_pair(key, _)));
      })
      .by(anony_cc(_ % 4))
    | coll::sort([](auto&& a, auto&& b) {
        return a.first < b.first || (a.first == b.first && a.second.first < b.second.first);
      })
    | coll::to<std::vector>();
  EXPECT_EQ(elems.size(), 100);
  for (int i = 0; i < 4; i++) {
    for (int j = i * 25, k = j + 25; j < k; j++) {
      EXPECT_EQ(elems[j].first, i);
      EXPECT_EQ(elems[j].second.second % 4, elems[i * 25].second.second % 4);
    }
    for (int j = i * 25, k = j + 25; j < k; j++) {
      EXPECT_EQ(elems[j].second.first, j - i * 25);
    }
  }
}

GTEST_TEST(Partition, Sort) {
  auto nums = coll::range(100)
    | coll::partition([](auto& key, auto in) {
        return in | coll::sort();
      })
      .by(anony_cc(_ % 5))
    | coll::to<std::vector>();
  EXPECT_EQ(nums.size(), 100);
  for (int i = 0; i < 5; i++) {
    for (int j = i * 20 + 1, k = j + 20 - 1; j < k; j++) {
      EXPECT_EQ(nums[j] % 5, nums[i * 20] % 5);
    }
    for (int j = i * 20 + 1, k = j + 20 - 1; j < k; j++) {
      EXPECT_LT(nums[j - 1], nums[j]);
    }
  }
}
