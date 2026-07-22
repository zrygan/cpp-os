#include "memory/PagingManager.h"
#include <gtest/gtest.h>

namespace PagingManagerConstruction {

// Sanity: a normal, valid construction works and produces the expected frame count
TEST(PagingManagerConstruction, ValidConfigProducesExpectedFrameCount) {
  prosched::PagingManager pm(16, 16384);
  EXPECT_EQ(pm.GetTotalFrameCount(), 1024);
}

// EXPECTED behavior (currently FAILS by crashing the test binary's child process):
// mem-per-frame of 0 must not be allowed to reach an unguarded integer division. This
// runs in a forked subprocess (gtest death-test) so a real crash here is contained and
// reported as a failure instead of taking down the whole test run.
TEST(PagingManagerConstruction, ZeroMemPerFrameShouldNotCrash) {
  EXPECT_EXIT(
      {
        prosched::PagingManager pm(0, 16384);
        std::exit(0);
      },
      ::testing::ExitedWithCode(0), ".*");
}

// EXPECTED behavior (currently FAILS): max-overall-mem smaller than mem-per-frame should
// not silently produce zero usable frames (integer division truncates 32/64 to 0) — a
// process would page-fault forever with no free frame ever available.
TEST(PagingManagerConstruction, OverallMemLessThanFrameSizeShouldHaveAtLeastOneFrame) {
  prosched::PagingManager pm(64, 32);
  EXPECT_GT(pm.GetTotalFrameCount(), 0);
}

} // namespace PagingManagerConstruction
