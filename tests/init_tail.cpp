#include "gtest/gtest.h"

#include "collection/coll.hpp"

#include "scapegoat.hpp"

class InitTail : public ::testing::Test {
public:
  inline static std::vector<int> ints;
  inline static std::vector<Scapegoat> goats;

protected:
  static void SetUpTestSuite() {
    coll::range(1000)
      | coll::map(anony_cc(rand() % 20))
      | coll::to(ints);
    coll::iterate(ints)
      | coll::map(anony_cc(Scapegoat{_}))
      | coll::to(goats);
  }

  static void TearDownTestSuite() {}
};


TEST_F(InitTail, Init) {
  auto r = coll::iterate(InitTail::ints) 
    | coll::init()
    | coll::filter([i = InitTail::ints.begin()](auto&& v) mutable {
        return *(i++) != v;
      })
    | coll::head();
  EXPECT_EQ(r, std::nullopt);
}

TEST_F(InitTail, InitGoat) {
  auto r = coll::iterate(InitTail::goats) 
    | coll::init()
    | coll::filter([i = InitTail::goats.begin()](auto&& v) mutable {
        return *(i++) != v;
      })
    | coll::head();
  EXPECT_EQ(r, std::nullopt);
}

TEST_F(InitTail, InitGoatRef) {
  ScapegoatCounter::clear();
  auto r = coll::iterate(InitTail::goats) 
    | coll::init().cache_by_ref()
    | coll::filter([i = InitTail::goats.begin()](auto&& v) mutable {
        return *(i++) != v;
      })
    | coll::head();
  EXPECT_EQ(r, std::nullopt);
  EXPECT_EQ(ScapegoatCounter::num_scapegoat_copy, 0);
  EXPECT_EQ(ScapegoatCounter::num_scapegoat_move, 0);
}

TEST_F(InitTail, InitReverse) {
  auto r = coll::iterate(InitTail::ints) 
    | coll::init()
    | coll::reverse()
    | coll::filter([i = InitTail::ints.rbegin()](auto&& v) mutable {
        return *(++i) != v;
      })
    | coll::head();
  EXPECT_EQ(r, std::nullopt);
}

TEST_F(InitTail, InitReverseGoat) {
  auto r = coll::iterate(InitTail::goats) 
    | coll::init()
    | coll::reverse()
    | coll::filter([i = InitTail::goats.rbegin()](auto&& v) mutable {
        return *(++i) != v;
      })
    | coll::head();
  EXPECT_EQ(r, std::nullopt);
}

TEST_F(InitTail, InitReverseGoatRef) {
  ScapegoatCounter::clear();
  auto r = coll::iterate(InitTail::goats) 
    | coll::init().cache_by_ref()
    | coll::reverse()
    | coll::filter([i = InitTail::goats.rbegin()](auto&& v) mutable {
        return *(++i) != v;
      })
    | coll::head();
  EXPECT_EQ(r, std::nullopt);
  EXPECT_EQ(ScapegoatCounter::num_scapegoat_copy, 0);
  EXPECT_EQ(ScapegoatCounter::num_scapegoat_move, 0);
}

TEST_F(InitTail, Tail) {
  auto r = coll::iterate(InitTail::ints) 
    | coll::tail()
    | coll::filter([i = InitTail::ints.begin()](auto&& v) mutable {
        return *(++i) != v;
      })
    | coll::head();
  EXPECT_EQ(r, std::nullopt);
}

TEST_F(InitTail, TailGoat) {
  auto r = coll::iterate(InitTail::goats) 
    | coll::tail()
    | coll::filter([i = InitTail::goats.begin()](auto&& v) mutable {
        return *(++i) != v;
      })
    | coll::head();
  EXPECT_EQ(r, std::nullopt);
}

TEST_F(InitTail, TailGoatRef) {
  ScapegoatCounter::clear();
  auto r = coll::iterate(InitTail::goats) 
    | coll::tail().cache_by_ref()
    | coll::filter([i = InitTail::goats.begin()](auto&& v) mutable {
        return *(++i) != v;
      })
    | coll::head();
  EXPECT_EQ(r, std::nullopt);
  EXPECT_EQ(ScapegoatCounter::num_scapegoat_copy, 0);
  EXPECT_EQ(ScapegoatCounter::num_scapegoat_move, 0);
}

TEST_F(InitTail, TailReverse) {
  auto r = coll::iterate(InitTail::ints) 
    | coll::tail()
    | coll::reverse()
    | coll::filter([i = InitTail::ints.rbegin()](auto&& v) mutable {
        return *(i++) != v;
      })
    | coll::head();
  EXPECT_EQ(r, std::nullopt);
}

TEST_F(InitTail, TailReverseGoat) {
  auto r = coll::iterate(InitTail::goats) 
    | coll::tail()
    | coll::reverse()
    | coll::filter([i = InitTail::goats.rbegin()](auto&& v) mutable {
        return *(i++) != v;
      })
    | coll::head();
  EXPECT_EQ(r, std::nullopt);
}

TEST_F(InitTail, TailReverseGoatRef) {
  ScapegoatCounter::clear();
  auto r = coll::iterate(InitTail::goats) 
    | coll::tail().cache_by_ref()
    | coll::reverse()
    | coll::filter([i = InitTail::goats.rbegin()](auto&& v) mutable {
        return *(i++) != v;
      })
    | coll::head();
  EXPECT_EQ(r, std::nullopt);
  EXPECT_EQ(ScapegoatCounter::num_scapegoat_copy, 0);
  EXPECT_EQ(ScapegoatCounter::num_scapegoat_move, 0);
}

TEST_F(InitTail, IterateSourceTail) {
  auto r = coll::generate([&, i = 0]() mutable -> auto& { return InitTail::ints[i++]; })
      .until([&, i = 0]() mutable { return i++ == InitTail::ints.size(); })
    | coll::tail()
    | coll::filter([i = InitTail::ints.begin()](auto&& v) mutable {
        return *(++i) != v;
      })
    | coll::head();
  EXPECT_EQ(r, std::nullopt);
}

TEST_F(InitTail, IterateSourceTailGoat) {
  auto r = coll::generate([&, i = 0]() mutable -> auto& { return InitTail::goats[i++]; })
      .until([&, i = 0]() mutable { return i++ == InitTail::goats.size(); })
    | coll::tail()
    | coll::filter([i = InitTail::goats.begin()](auto&& v) mutable {
        return *(++i) != v;
      })
    | coll::head();
  EXPECT_EQ(r, std::nullopt);
}

TEST_F(InitTail, IterateSourceTailGoatRef) {
  ScapegoatCounter::clear();
  auto r = coll::generate([&, i = 0]() mutable -> auto& { return InitTail::goats[i++]; })
      .until([&, i = 0]() mutable { return i++ == InitTail::goats.size(); })
    | coll::tail().cache_by_ref()
    | coll::filter([i = InitTail::goats.begin()](auto&& v) mutable {
        return *(++i) != v;
      })
    | coll::head();
  EXPECT_EQ(r, std::nullopt);
  EXPECT_EQ(ScapegoatCounter::num_scapegoat_copy, 0);
  EXPECT_EQ(ScapegoatCounter::num_scapegoat_move, 0);
}

TEST_F(InitTail, IterateSourceTailReverseGoatRef) {
  auto r = coll::generate([&, i = 0]() mutable -> auto& { return InitTail::goats[i++]; })
      .until([&, i = 0]() mutable { return i++ == InitTail::goats.size(); })
    | coll::tail().cache_by_ref()
    | coll::reverse().with_buffer()
    | coll::filter([i = InitTail::goats.rbegin()](auto&& v) mutable {
        return *(i++) != v;
      })
    | coll::head();
  EXPECT_EQ(r, std::nullopt);
}

TEST_F(InitTail, IterateSourceTailReverseRefGoatRef) {
  ScapegoatCounter::clear();
  auto r = coll::generate([&, i = 0]() mutable -> auto& { return InitTail::goats[i++]; })
      .until([&, i = 0]() mutable { return i++ == InitTail::goats.size(); })
    | coll::tail().cache_by_ref()
    | coll::reverse().with_buffer().cache_by_ref()
    | coll::filter([i = InitTail::goats.rbegin()](auto&& v) mutable {
        return *(i++) != v;
      })
    | coll::head();
  EXPECT_EQ(r, std::nullopt);
  EXPECT_EQ(ScapegoatCounter::num_scapegoat_copy, 0);
  EXPECT_EQ(ScapegoatCounter::num_scapegoat_move, 0);
}
