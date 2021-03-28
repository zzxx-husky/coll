#include <set>
#include <unordered_set>
#include <vector>

#include "gtest/gtest.h"

#include "collection/coll.hpp"

#include "scapegoat.hpp"

class Distinct : public ::testing::Test {
public:
  inline static std::vector<Scapegoat> goats;
  inline static std::vector<int> distinct_ints;
  inline static std::vector<int> ints;

protected:
  static void SetUpTestSuite() {
    auto size = 1000;
    coll::range(size)
      | coll::map(anonyr_cc(rand() % size))
      | coll::to(ints);

    coll::iterate(std::set<int>(ints.begin(), ints.end()))
      | coll::to(distinct_ints);

    goats = coll::iterate(ints)
      | coll::map(anony_cc(Scapegoat{_}))
      | coll::to(goats);
  }

  static void TearDownTestSuite() {}
};

TEST_F(Distinct, Basic) {
  auto v = coll::iterate(Distinct::ints)
    | coll::distinct()
    | coll::sort()
    | coll::to<std::vector>();

  EXPECT_EQ(v.size(), Distinct::distinct_ints.size());
  EXPECT_EQ(v, Distinct::distinct_ints);
}

TEST_F(Distinct, UsingUnorderedSet) {
  auto v = coll::iterate(Distinct::ints)
    | coll::distinct<std::unordered_set>()
    | coll::sort()
    | coll::to<std::vector>();

  EXPECT_EQ(v.size(), Distinct::distinct_ints.size());
  EXPECT_EQ(v, Distinct::distinct_ints);
}

TEST_F(Distinct, UsingSet) {
  auto v = coll::iterate(Distinct::ints)
    | coll::distinct<std::set>()
    | coll::sort()
    | coll::to<std::vector>();

  EXPECT_EQ(v.size(), Distinct::distinct_ints.size());
  EXPECT_EQ(v, Distinct::distinct_ints);
}

TEST_F(Distinct, UsingVector) {
  auto v = coll::iterate(Distinct::ints)
    | coll::distinct<std::vector>([](auto& set, auto&& e) -> bool {
        if (std::find(set.begin(), set.end(), e) == set.end()) {
          set.push_back(e);
          return true;
        }
        return false;
      })
    | coll::sort()
    | coll::to<std::vector>();

  EXPECT_EQ(v.size(), Distinct::distinct_ints.size());
  EXPECT_EQ(v, Distinct::distinct_ints);
}

TEST_F(Distinct, UsingGoat) {
  ScapegoatCounter::clear();
  auto v = coll::iterate(Distinct::goats)
    | coll::distinct<std::unordered_set>()
    | coll::map(anony_rr(_.val))
    | coll::sort()
    | coll::to<std::vector>();
  EXPECT_EQ(v, Distinct::distinct_ints);
}

TEST_F(Distinct, UsingGoatRef) {
  ScapegoatCounter::clear();
  auto v = coll::iterate(Distinct::goats)
    | coll::distinct<std::unordered_set>()
      .cache_by_ref()
    | coll::map(anony_rr(_.val))
    | coll::sort()
    | coll::to<std::vector>();
  EXPECT_EQ(v, Distinct::distinct_ints);
  EXPECT_EQ(ScapegoatCounter::num_scapegoat_copy, 0);
  EXPECT_EQ(ScapegoatCounter::num_scapegoat_move, 0);
}
