#include "coll/coll.hpp"
#include "gtest/gtest.h"

GTEST_TEST(Traversal, Basic) {
  {
    auto v = coll::range(10)
      | coll::to_traversal()
      | coll::to_vector();

    EXPECT_EQ(v, coll::range(10) | coll::to_vector());
  }
  {
    auto v = coll::range(10)
      | coll::to_traversal()
      | coll::map(anony_cc(_ * 2))
      | coll::to_traversal()
      | coll::to_vector();
    EXPECT_EQ(v, coll::range(0, 20, 2) | coll::to_vector());
  }
}

GTEST_TEST(Traversal, Vector) {
  auto traversals = coll::range(10)
    | coll::map(anony_cc(
        coll::elements(_)
          | coll::map(anony_cc(_ * 2))
          | coll::to_traversal()
      ))
    | coll::to_vector();

  auto v = coll::iterate(traversals)
    | coll::map(anony_rc(
        _ | coll::head() | coll::unwrap()
      ))
    | coll::to_vector();

  EXPECT_EQ(v, coll::range(0, 20, 2) | coll::to_vector());
}

GTEST_TEST(Traversal, Combine) {
  auto a = coll::range(10)
    | coll::map(anony_cc(_ + 1))
    | coll::to_traversal();

  auto b = coll::range(10)
    | coll::map(anony_cc(_ * 2))
    | coll::to_traversal();

  auto c = coll::range(10)
    | coll::map(anony_cc(_ - 3))
    | coll::to_traversal();

  auto d = coll::range(10)
    | coll::map(anony_cc(_ / 4))
    | coll::to_traversal();

  auto v = coll::elements(a, b, c, d)
    | coll::map(anony_rc(
        _ | coll::sum() | coll::unwrap()
      ))
    | coll::to_vector();

  EXPECT_EQ(v, (std::vector<int>{
    coll::range(10) | coll::map(anony_cc(_ + 1)) | coll::sum() | coll::unwrap(),
    coll::range(10) | coll::map(anony_cc(_ * 2)) | coll::sum() | coll::unwrap(),
    coll::range(10) | coll::map(anony_cc(_ - 3)) | coll::sum() | coll::unwrap(),
    coll::range(10) | coll::map(anony_cc(_ / 4)) | coll::sum() | coll::unwrap()
  }));
}

GTEST_TEST(Traversal, Reverse) {
  auto a = coll::range(10)
    | coll::to_traversal();

  EXPECT_EQ(a | coll::reverse().with_buffer() | coll::to_vector(),
    coll::range(10) | coll::reverse() | coll::to_vector());
}

GTEST_TEST(Traversal, Break) {
  {
    auto a = coll::range(10)
      | coll::take_while(anony_cc(_ < 3))
      | coll::to_traversal();

    EXPECT_EQ(a | coll::sum() | coll::unwrap(), 0 + 1 + 2);
  }

  {
    auto a = coll::range(10)
      | coll::to_traversal();

    EXPECT_EQ(
      a | coll::take_while(anony_cc(_ < 3)) | coll::sum() | coll::unwrap(),
      0 + 1 + 2
    );
  }
}
