#include "coll/coll.hpp"
#include "gtest/gtest.h"

#include "scapegoat.hpp"

class Window : public ::testing::Test {
public:
  inline static std::vector<Scapegoat> vals;

protected:
  static void SetUpTestSuite() {
    vals = coll::range(20)
      | coll::map(anony_cc(Scapegoat{rand() % 20}))
      | coll::to<std::vector>();
  }
};

TEST_F(Window, 1) {
  ScapegoatCounter::clear();
  int num_windows = 0;
  coll::iterate(Window::vals)
    | coll::window(1)
    | coll::foreach([&, i = 0](auto&& w) mutable {
        ++num_windows;
        EXPECT_EQ(w.size(), 1);
        EXPECT_EQ(w[0], Window::vals[i++]);
      });
  EXPECT_EQ(num_windows, 20);
}

TEST_F(Window, 5) {
  ScapegoatCounter::clear();
  int num_windows = 0;
  coll::iterate(Window::vals)
    | coll::window(5)
    | coll::foreach([&, i = 0](auto&& w) mutable {
        ++num_windows;
        EXPECT_EQ(w.size(), 5);
        for (auto& x : w) {
          EXPECT_EQ(x, Window::vals[i++]);
        }
      });
  EXPECT_EQ(num_windows, 20 / 5);
}

TEST_F(Window, 5_1) {
  ScapegoatCounter::clear();
  int num_windows = 0;
  coll::iterate(Window::vals)
    | coll::window(5, 1)
    | coll::foreach([&, i = 0](auto&& w) mutable {
        ++num_windows;
        EXPECT_EQ(w.size(), 5);
        for (int j = 0; j < 5; j++) {
          EXPECT_EQ(w[j], Window::vals[i + j]);
        }
        i++;
      });
  EXPECT_EQ(num_windows, 20 - 5 + 1);
}

TEST_F(Window, 5_1_REF) {
  ScapegoatCounter::clear();
  int num_windows = 0;
  coll::iterate(Window::vals)
    | coll::window(5, 1).cache_by_ref()
    | coll::foreach([&, i = 0](auto&& w) mutable {
        ++num_windows;
        EXPECT_EQ(w.size(), 5);
        for (int j = 0; j < 5; j++) {
          EXPECT_EQ(w[j], Window::vals[i + j]);
        }
        i++;
      });
  EXPECT_EQ(num_windows, 20 - 5 + 1);
  EXPECT_EQ(ScapegoatCounter::num_scapegoat_copy, 0);
  EXPECT_EQ(ScapegoatCounter::num_scapegoat_move, 0);
}

TEST_F(Window, 5_4_REF) {
  ScapegoatCounter::clear();
  int num_windows = 0;
  coll::iterate(Window::vals)
    | coll::window(5, 4).cache_by_ref()
    | coll::foreach([&, i = 0](auto&& w) mutable {
        ++num_windows;
        auto n = i + 5 <= 20 ? 5 : 20 - i;
        EXPECT_EQ(w.size(), n);
        for (int j = 0; j < n; j++) {
          EXPECT_EQ(w[j], Window::vals[i + j]);
        }
        i += 4;
      });
  EXPECT_EQ(num_windows, (20 - 1) / 4 + ((20 - 1) % 4 != 0));
  EXPECT_EQ(ScapegoatCounter::num_scapegoat_copy, 0);
  EXPECT_EQ(ScapegoatCounter::num_scapegoat_move, 0);
}

TEST_F(Window, 5_4_REF_REV) {
  ScapegoatCounter::clear();
  int num_windows = 0;
  coll::iterate(Window::vals)
    | coll::window(5, 4).cache_by_ref()
    | coll::reverse().with_buffer()
    | coll::foreach([&, n = (20 - 1) % 4 + 1, i = 20 - ((20 - 1) % 4 + 1)](auto&& w) mutable {
        ++num_windows;
        EXPECT_EQ(w.size(), n);
        for (int j = 0; j < n; j++) {
          EXPECT_EQ(w[j], Window::vals[i + j]);
        }
        n = 5;
        i -= 4;
      });
  EXPECT_EQ(num_windows, (20 - 1) / 4 + ((20 - 1) % 4 != 0));
  EXPECT_EQ(ScapegoatCounter::num_scapegoat_copy, 0);
  EXPECT_EQ(ScapegoatCounter::num_scapegoat_move, 0);
}

TEST_F(Window, 5_4_VAL_REV) {
  ScapegoatCounter::clear();
  int num_windows = 0;
  coll::iterate(Window::vals)
    | coll::window(5, 4)
    | coll::reverse().with_buffer()
    | coll::foreach([&, n = (20 - 1) % 4 + 1, i = 20 - ((20 - 1) % 4 + 1)](auto&& w) mutable {
        ++num_windows;
        EXPECT_EQ(w.size(), n);
        for (int j = 0; j < n; j++) {
          EXPECT_EQ(w[j], Window::vals[i + j]);
        }
        n = 5;
        i -= 4;
      });
  EXPECT_EQ(num_windows, (20 - 1) / 4 + ((20 - 1) % 4 != 0));
}

TEST_F(Window, 5_6_REF_REV) {
  ScapegoatCounter::clear();
  int num_windows = 0;
  coll::iterate(Window::vals)
    | coll::window(5, 6).cache_by_ref()
    | coll::reverse().with_buffer()
    | coll::foreach([&, n = 20 % 6, i = 20 - (20 % 6)](auto&& w) mutable {
        ++num_windows;
        EXPECT_EQ(w.size(), n);
        for (int j = 0; j < n; j++) {
          EXPECT_EQ(w[j], Window::vals[i + j]);
        }
        n = 5;
        i -= 6;
      });
  EXPECT_EQ(num_windows, 20 / 6 + (20 % 6 != 0));
  EXPECT_EQ(ScapegoatCounter::num_scapegoat_copy, 0);
  EXPECT_EQ(ScapegoatCounter::num_scapegoat_move, 0);
}
