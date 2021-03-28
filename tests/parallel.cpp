#include "collection/coll.hpp"
#include "gtest/gtest.h"

GTEST_TEST(Parallel, Sum) {
  int n = 4;
  int es = (0 + 100 - 1) * 100 / 2;
  std::vector<int> ts(n, 0);

  int s = *(coll::range(100)
    | coll::parallel(n, [&](size_t pid, auto in) {
        return in
          | coll::inspect([&, pid](auto&& x) { ts[pid] += x; })
          | coll::sum();
      })
    | coll::map(anony_ac(_.second.value_or(0)))
    | coll::sum());

  EXPECT_EQ(s, es);
  EXPECT_EQ(coll::iterate(ts) | coll::sum(), es);
}

GTEST_TEST(Parallel, HeadSum) {
  int n = 4;
  int es = (0 + 100 - 1) * 100 / 2;
  std::vector<int> ts(n, -1);

  int s = *(coll::range(100)
    | coll::parallel(n, [&](size_t pid, auto in) {
        return in
          | coll::inspect([&, pid](auto&& x) {
              if (ts[pid] == -1) { ts[pid] = x; }
            })
          | coll::head();
      })
    | coll::map(anony_ac(_.second.value_or(0)))
    | coll::sum());

  EXPECT_EQ(*(coll::iterate(ts) | coll::sum()), s);
}

GTEST_TEST(Parallel, DoubleSum) {
  int n = 4;
  int es = (0 + 100 - 1) * 100 / 2;

  int s = *(coll::range(100)
    | coll::parallel(n, [](size_t pid, auto in) {
        return in | coll::map(anony_cc(_ * 2));
      })
    | coll::sum());

  EXPECT_EQ(es * 2, s);
}

GTEST_TEST(Parallel, DoubleSumFewInts) {
  int n = 4;
  int es = (0 + 3 - 1) * 3 / 2;

  int s = *(coll::range(3)
    | coll::parallel(n, [](size_t pid, auto in) {
        return in | coll::map(anony_cc(_ * 2));
      })
    | coll::sum());

  EXPECT_EQ(es * 2, s);
}

GTEST_TEST(Parallel, NoResult) {
  int n = 4;
  std::vector<std::vector<int>> ps(n);

  auto cnt = coll::range(8)
    | coll::parallel(n, [&](size_t pid, auto in) {
        return in | coll::foreach([&, pid](auto&& e) {
          ps[pid].push_back(e);
        });
      })
    | coll::map(anonyr_aa(ps[_].size()))
    | coll::sum();

  EXPECT_EQ(cnt, 8);
}

GTEST_TEST(Parallel, ParallelPartition) {
  int n = 4;
  int es = (0 + 100 - 1) * 100 / 2;

  int s = *(coll::range(100)
    | coll::parallel(n, [](size_t pid, auto in) {
        return in
          | coll::partition([](int key, auto in) {
              return in | coll::sum();
            })
            .by(anony_cc(_ % 8));
      })
      .shuffle_by(coll::shuffle::Partition(anony_cc(_ % 8)))
    | coll::map(anony_ac(*_.second))
    | coll::sum());

  EXPECT_EQ(es, s);
}

GTEST_TEST(Parallel, ParallelPartitionAlias) {
  int n = 4;
  int es = (0 + 100 - 1) * 100 / 2;

  int s = *(coll::range(100)
    | coll::parallel_partition(n, [](int key, auto in) {
        return in | coll::sum();
      })
      .key_by(anony_cc(_ % 8))
    | coll::map(anony_ac(*_.second))
    | coll::sum());

  EXPECT_EQ(es, s);
}
