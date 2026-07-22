#include "memory/PagingManager.h"
#include <gtest/gtest.h>

namespace PagingManagerConstruction {

// A normal, valid construction produces the expected frame count
TEST(PagingManagerConstruction, ValidConfigProducesExpectedFrameCount) {
  prosched::PagingManager pm(16, 16384);
  EXPECT_EQ(pm.GetTotalFrameCount(), 1024);
}

// mem-per-frame of 0 should not reach an unguarded division by zero (run as a death
// test so a crash is contained instead of killing the whole test binary)
TEST(PagingManagerConstruction, ZeroMemPerFrameShouldNotCrash) {
  EXPECT_EXIT(
      {
        prosched::PagingManager pm(0, 16384);
        std::exit(0);
      },
      ::testing::ExitedWithCode(0), ".*");
}

// max-overall-mem smaller than mem-per-frame should not yield zero usable frames
TEST(PagingManagerConstruction, OverallMemLessThanFrameSizeShouldHaveAtLeastOneFrame) {
  prosched::PagingManager pm(64, 32);
  EXPECT_GT(pm.GetTotalFrameCount(), 0);
}

} // namespace PagingManagerConstruction
