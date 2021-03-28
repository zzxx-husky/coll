#include <algorithm>
#include <numeric>
#include <vector>

#include "gtest/gtest.h"

#include "collection/coll.hpp"

#include "scapegoat.hpp"

class Sink : public ::testing::Test {
public:
  inline static std::vector<int> ints;
  inline static std::vector<Scapegoat> goats;

protected:
  static void SetUpTestSuite() {
    auto size = 1000;
    coll::range(size)
      | coll::map(anonyr_cc(rand() % size))
      | coll::to(ints);
    coll::iterate(ints)
      | coll::map(anony_cc(Scapegoat{_}))
      | coll::to(goats);
  }

  static void TearDownTestSuite() {}
};

TEST_F(Sink, Count) {
  EXPECT_EQ(Sink::ints.size(),
    coll::iterate(Sink::ints) | coll::count()
  );
}

TEST_F(Sink, Max) {
  EXPECT_EQ(*std::max_element(Sink::ints.begin(), Sink::ints.end()),
    *(coll::iterate(Sink::ints) | coll::max())
  );
}

TEST_F(Sink, MaxGoat) {
  EXPECT_EQ(std::max_element(Sink::goats.begin(), Sink::goats.end())->val,
    (coll::iterate(Sink::goats) | coll::max())->val
  );

  ScapegoatCounter::clear();
  EXPECT_EQ(std::max_element(Sink::goats.begin(), Sink::goats.end())->val,
    (coll::iterate(Sink::goats) | coll::max().ref())->val
  );
  EXPECT_EQ(ScapegoatCounter::num_scapegoat_copy, 0);
  EXPECT_EQ(ScapegoatCounter::num_scapegoat_move, 0);
}

TEST_F(Sink, Min) {
  EXPECT_EQ(*std::min_element(Sink::ints.begin(), Sink::ints.end()),
    *(coll::iterate(Sink::ints) | coll::min())
  );
}

TEST_F(Sink, MinGoat) {
  EXPECT_EQ(std::min_element(Sink::goats.begin(), Sink::goats.end())->val,
    (coll::iterate(Sink::goats) | coll::min())->val
  );

  ScapegoatCounter::clear();
  EXPECT_EQ(std::min_element(Sink::goats.begin(), Sink::goats.end())->val,
    (coll::iterate(Sink::goats) | coll::min().ref())->val
  );
  EXPECT_EQ(ScapegoatCounter::num_scapegoat_copy, 0);
  EXPECT_EQ(ScapegoatCounter::num_scapegoat_move, 0);
}

TEST_F(Sink, Sum) {
  EXPECT_EQ(std::accumulate(Sink::ints.begin(), Sink::ints.end(), 0),
    *(coll::iterate(Sink::ints) | coll::sum())
  );
}

TEST_F(Sink, SumGoat) {
  EXPECT_EQ(std::accumulate(Sink::goats.begin(), Sink::goats.end(), Scapegoat{0}),
    *(coll::iterate(Sink::goats) | coll::sum())
  );
}

TEST_F(Sink, Avg) {
  EXPECT_EQ(std::accumulate(Sink::ints.begin(), Sink::ints.end(), 0) / Sink::ints.size(),
    *(coll::iterate(Sink::ints) | coll::avg())
  );
}

TEST_F(Sink, AvgDouble) {
  EXPECT_EQ(std::accumulate(Sink::ints.begin(), Sink::ints.end(), 0.) / Sink::ints.size(),
    *(coll::iterate(Sink::ints) | coll::map(anony_cc(double(_))) | coll::avg())
  );
}

TEST_F(Sink, Head) {
  EXPECT_EQ(Sink::ints.front(),
    *(coll::iterate(Sink::ints) | coll::head())
  );
}

TEST_F(Sink, HeadGoat) {
  EXPECT_EQ(Sink::goats.front(),
    *(coll::iterate(Sink::goats) | coll::head())
  );

  ScapegoatCounter::clear();
  EXPECT_EQ(Sink::goats.front(),
    *(coll::iterate(Sink::goats) | coll::head().ref())
  );
  EXPECT_EQ(ScapegoatCounter::num_scapegoat_copy, 0);
  EXPECT_EQ(ScapegoatCounter::num_scapegoat_move, 0);
}

TEST_F(Sink, Last) {
  EXPECT_EQ(Sink::ints.back(),
    *(coll::iterate(Sink::ints) | coll::last())
  );
}

TEST_F(Sink, LastGoat) {
  EXPECT_EQ(Sink::goats.back(),
    *(coll::iterate(Sink::goats) | coll::last())
  );

  ScapegoatCounter::clear();
  EXPECT_EQ(Sink::goats.back(),
    *(coll::iterate(Sink::goats) | coll::last().ref())
  );
  EXPECT_EQ(ScapegoatCounter::num_scapegoat_copy, 0);
  EXPECT_EQ(ScapegoatCounter::num_scapegoat_move, 0);
}
