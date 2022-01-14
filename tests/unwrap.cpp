#include <optional>
#include <vector>

#include "coll/coll.hpp"
#include "gtest/gtest.h"

GTEST_TEST(Unwrap, Objects) {
  {
    std::optional<int> a = std::nullopt;
    EXPECT_THROW(a | coll::unwrap(), std::runtime_error);

    a = 12;
    EXPECT_EQ(a | coll::unwrap(), *a);
  }
  {
    auto pipe = coll::range(10) | coll::filter(anony_cc(_ % 2 == 0));
    EXPECT_EQ(pipe | coll::min() | coll::unwrap(), 0);
    EXPECT_EQ(pipe | coll::max() | coll::unwrap(), 8);
    EXPECT_EQ(pipe | coll::sum() | coll::unwrap(), 20);
    EXPECT_THROW(
      pipe | coll::filter(anony_cc(_ % 2 == 1)) | coll::sum() | coll::unwrap(),
      std::runtime_error);
  }
  {
    int* x = nullptr;
    EXPECT_THROW(x | coll::unwrap(), std::runtime_error);

    int a = 23;
    x = &a;
    EXPECT_EQ(x | coll::unwrap(), a);
  }
  {
    std::vector<int> y{1};
    auto x = y.begin();
    EXPECT_EQ(x | coll::unwrap(), y[0]);
  }
}

GTEST_TEST(Unwrap, Coll) {
  {
    auto sum = coll::range(10)
      | coll::map(anony_cc(std::optional(_)))
      | coll::unwrap()
      | coll::sum()
      | coll::unwrap();
    EXPECT_EQ(sum, (0 + 9) * 10 / 2);
  }
  {
    auto vec = coll::range(10) | coll::to_vector();
    auto sum = coll::iterate(vec)
      | coll::map(anony_rc(&_))
      | coll::unwrap()
      | coll::sum()
      | coll::unwrap();
    EXPECT_EQ(sum, (0 + 9) * 10 / 2);
  }
  {
    auto vec = coll::range(10) | coll::to_vector();
    auto sum = coll::generate([i = vec.begin()]() mutable { return i++; }).times(vec.size())
      | coll::unwrap()
      | coll::sum()
      | coll::unwrap(); 
    EXPECT_EQ(sum, (0 + 9) * 10 / 2);
  }
}

GTEST_TEST(UnwrapOr, Basic) {
  {
    auto val = coll::range(0, 0)
      | coll::min()
      | coll::unwrap_or(10);

    EXPECT_EQ(val, 10);
  }
  {
    auto val = coll::range(0, 10)
      | coll::filter(anony_cc(_ % 2 == 1))
      | coll::head()
      | coll::unwrap_or(10);

    EXPECT_EQ(val, 1);
  }
}
