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

// ─── PagingManagerBasic ───────────────────────────────────────────────────

namespace PagingManagerBasic {

// PageIn brings a page into a free frame and makes it resident
TEST(PagingManagerBasic, PageInLoadsPageIntoFrame) {
  prosched::PagingManager pm(16, 64);
  EXPECT_TRUE(pm.PageIn(1, 0));
  EXPECT_TRUE(pm.IsPageResident(1, 0));
  EXPECT_GE(pm.GetFrame(1, 0), 0);
}

// Paging in the same page twice is idempotent — only one resident page results
TEST(PagingManagerBasic, PageInSamePageTwiceIsIdempotent) {
  prosched::PagingManager pm(16, 64);
  EXPECT_TRUE(pm.PageIn(1, 0));
  EXPECT_TRUE(pm.PageIn(1, 0));
  EXPECT_EQ(pm.GetResidentPageCount(1), 1);
}

// Resident page count and process byte usage track the pages a process holds
TEST(PagingManagerBasic, ResidentPageCountAndProcessBytesTrackPages) {
  prosched::PagingManager pm(16, 64);
  pm.PageIn(1, 0);
  pm.PageIn(1, 1);
  EXPECT_EQ(pm.GetResidentPageCount(1), 2);
  EXPECT_EQ(pm.GetProcessMemoryBytes(1), 32u);
}

// Memory stats reflect a single allocation against the total frame pool
TEST(PagingManagerBasic, MemoryStatsReflectSingleAllocation) {
  prosched::PagingManager pm(16, 64);
  pm.PageIn(1, 0);
  auto stats = pm.GetMemoryStats();
  EXPECT_EQ(stats.totalFrames, 4);
  EXPECT_EQ(stats.usedFrames, 1);
  EXPECT_EQ(stats.freeFrames, 3);
  EXPECT_EQ(stats.usedMemoryBytes, 16u);
  EXPECT_EQ(stats.pagesPagedIn, 1u);
}

// Freeing a process releases its frames and clears its page table
TEST(PagingManagerBasic, FreeAllPagesReleasesFramesAndPages) {
  prosched::PagingManager pm(16, 64);
  pm.PageIn(1, 0);
  pm.PageIn(1, 1);
  pm.FreeAllPagesForProcess(1);
  EXPECT_EQ(pm.GetResidentPageCount(1), 0);
  EXPECT_FALSE(pm.IsPageResident(1, 0));
  EXPECT_EQ(pm.GetMemoryStats().usedFrames, 0);
}

// With only one frame, paging in a second page evicts the first (FIFO)
TEST(PagingManagerBasic, FifoEvictionReusesFrameForNewPage) {
  prosched::PagingManager pm(16, 16);
  EXPECT_TRUE(pm.PageIn(1, 0));
  EXPECT_TRUE(pm.PageIn(1, 1));
  EXPECT_TRUE(pm.IsPageResident(1, 1));
  EXPECT_FALSE(pm.IsPageResident(1, 0));
  EXPECT_EQ(pm.GetMemoryStats().pagesPagedOut, 1u);
}

// A frame snapshot reports which process and page own each resident frame
TEST(PagingManagerBasic, FrameSnapshotShowsResidentOwnership) {
  prosched::PagingManager pm(16, 64);
  pm.PageIn(1, 0);
  bool found = false;
  for (const auto &frame : pm.GetFrameSnapshot()) {
    if (frame.allocated) {
      EXPECT_EQ(frame.pid, 1);
      EXPECT_EQ(frame.pageNumber, 0);
      found = true;
    }
  }
  EXPECT_TRUE(found);
}

} // namespace PagingManagerBasic
