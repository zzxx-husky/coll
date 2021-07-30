#include "gtest/gtest.h"

#include "coll/coll.hpp"

#include "scapegoat.hpp"

class Sort : public ::testing::Test {
public:
  inline static std::vector<Scapegoat> vals;
  inline static std::vector<int> sorted_ints;

protected:
  static void SetUpTestSuite() {
    coll::range(20)
      | coll::map(anony_cc(Scapegoat{rand() % 20}))
      | coll::to(vals);

    coll::iterate(vals)
      | coll::map(anony_rc(_.val))
      | coll::sort()
      | coll::to(sorted_ints);
  }

  static void TearDownTestSuite() {}
};

TEST_F(Sort, Basic) {
  ScapegoatCounter::clear();
  auto sorted = coll::iterate(Sort::vals)
    | coll::sort()
    | coll::map(anony_rr(_.val))
    | coll::to<std::vector>();
  EXPECT_EQ(sorted, Sort::sorted_ints);
}

TEST_F(Sort, SortWithRef) {
  ScapegoatCounter::clear();
  auto sorted = coll::iterate(Sort::vals)
    | coll::sort().cache_by_ref()
    | coll::map(anony_rr(_.val))
    | coll::to<std::vector>();
  EXPECT_EQ(sorted, Sort::sorted_ints);
  EXPECT_EQ(ScapegoatCounter::num_scapegoat_copy, 0);
  EXPECT_EQ(ScapegoatCounter::num_scapegoat_move, 0);
}

TEST_F(Sort, ReverseSort) {
  ScapegoatCounter::clear();
  auto sorted = coll::iterate(Sort::vals)
    | coll::sort().reverse()
    | coll::map(anony_rr(_.val))
    | coll::to<std::vector>();
  std::reverse(sorted.begin(), sorted.end());
  EXPECT_EQ(sorted, Sort::sorted_ints);
}

TEST_F(Sort, SortByMember) {
  ScapegoatCounter::clear();
  auto sorted = coll::iterate(Sort::vals)
    | coll::sort(anony_rc(_.val))
    | coll::map(anony_rr(_.val))
    | coll::to<std::vector>();
  EXPECT_EQ(sorted, Sort::sorted_ints);
}

TEST_F(Sort, ReverseSortByReverseComparator) {
  ScapegoatCounter::clear();
  auto sorted = coll::iterate(Sort::vals)
    | coll::sort([](auto& a, auto& b) { return b < a; })
        .reverse()
    | coll::map(anony_rr(_.val))
    | coll::to<std::vector>();
  EXPECT_EQ(sorted, Sort::sorted_ints);
}

TEST_F(Sort, ReverseSortByRef) {
  ScapegoatCounter::clear();
  auto sorted = coll::iterate(Sort::vals)
    | coll::sort([](auto& a, auto& b) { return b < a; })
        .cache_by_ref()
    | coll::map(anony_rr(_.val))
    | coll::to<std::vector>();
  std::reverse(sorted.begin(), sorted.end());
  EXPECT_EQ(sorted, Sort::sorted_ints);
  EXPECT_EQ(ScapegoatCounter::num_scapegoat_copy, 0);
  EXPECT_EQ(ScapegoatCounter::num_scapegoat_move, 0);
}

TEST_F(Sort, ReverseSortThenReverse) {
  ScapegoatCounter::clear();
  auto sorted = coll::iterate(Sort::vals)
    | coll::sort().reverse().cache_by_ref()
    | coll::reverse()
    | coll::map(anony_rr(_.val))
    | coll::to<std::vector>();
  EXPECT_EQ(sorted, Sort::sorted_ints);
  EXPECT_EQ(ScapegoatCounter::num_scapegoat_copy, 0);
  EXPECT_EQ(ScapegoatCounter::num_scapegoat_move, 0);
}

TEST_F(Sort, ReverseSortByRefThenReverse) {
  ScapegoatCounter::clear();
  auto sorted = coll::iterate(Sort::vals)
    | coll::sort([](auto& a, auto& b) { return b < a; })
        .cache_by_ref()
    | coll::reverse()
    | coll::map(anony_rr(_.val))
    | coll::to<std::vector>();
  EXPECT_EQ(sorted, Sort::sorted_ints);
  EXPECT_EQ(ScapegoatCounter::num_scapegoat_copy, 0);
  EXPECT_EQ(ScapegoatCounter::num_scapegoat_move, 0);
}
