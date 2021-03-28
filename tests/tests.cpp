#include "gtest/gtest.h"

/**
 * The entry to start the test of Core module
 */
GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
