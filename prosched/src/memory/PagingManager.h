#pragma once

#include <cstddef>
#include <map>
#include <vector>

namespace prosched {

class PagingManager {
public:
  struct Frame {
    int frameNumber = -1;
    std::size_t sizeBytes = 0;
    bool allocated = false;
  };

  struct PageTableEntry {
    bool resident = false;
    int frameNumber = -1;
  };

  /**
   * @brief Constructs a new PagingManager instance.
   *
   * @param memPerFrame The amount of memory per frame.
   * @param maxOverallMem The maximum overall memory size.
   */
  PagingManager(int memPerFrame, int maxOverallMem)
      : memPerFrame(memPerFrame), maxOverallMem(maxOverallMem), totalFrames(maxOverallMem / memPerFrame) {
    frames.resize(totalFrames);
    for (int i = 0; i < totalFrames; ++i) {
      frames[i].frameNumber = i;
      frames[i].sizeBytes = static_cast<std::size_t>(memPerFrame);
      frames[i].allocated = false;
    }
  }
  /**
   * @brief Checks if a page is currently resident in physical memory.
   *
   * @param pid The process ID.
   * @param pageNum The page number.
   * @return True if the page is resident, false otherwise.
   */
  bool IsPageResident(int pid, int pageNum) {
    (void)pid;
    (void)pageNum;
    return false;
  }

  /**
   * @brief Gets the frame number for a given process and page.
   *
   * @param pid The process ID.
   * @param pageNum The page number.
   * @return The frame number if the page is resident, -1 otherwise.
   */
  int GetFrame(int pid, int pageNum) {
    (void)pid;
    (void)pageNum;
    return -1;
  }

  /**
   * @brief Pages in a page into physical memory.
   *
   * @param pid The process ID.
   * @param pageNum The page number.
   * @return True if the page was successfully paged in, false otherwise.
   */
  bool PageIn(int pid, int pageNum) {
    (void)pid;
    (void)pageNum;
    return false;
  }

  /**
   * @brief Gets the total number of frames.
   *
   * @return The total number of frames.
   */
  int GetTotalFrameCount() const {
    return totalFrames;
  }

private:
  int memPerFrame = 0;
  int maxOverallMem = 0;
  int totalFrames = 0;
  std::vector<Frame> frames;
  std::map<int, std::map<int, PageTableEntry>> pageTables;
};

}  // namespace prosched
