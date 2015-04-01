#include "gtest/gtest.h"

// This is the entry point for the CLI executable that runs all the monitor's
// unit tests.
int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
