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
    auto pidIt = pageTables.find(pid);
    if (pidIt == pageTables.end()) {
      return false;
    }

    auto pageIt = pidIt->second.find(pageNum);
    if (pageIt == pidIt->second.end()) {
      return false;
    }

    return pageIt->second.resident;
  }

  /**
   * @brief Gets the frame number for a given process and page.
   *
   * @param pid The process ID.
   * @param pageNum The page number.
   * @return The frame number if the page is resident, -1 otherwise.
   */
  int GetFrame(int pid, int pageNum) {
    auto pidIt = pageTables.find(pid);
    if (pidIt == pageTables.end()) {
      return -1;
    }

    auto pageIt = pidIt->second.find(pageNum);
    if (pageIt == pidIt->second.end() || !pageIt->second.resident) {
      return -1;
    }

    return pageIt->second.frameNumber;
  }

  /**
   * @brief Pages in a page into physical memory.
   * @note This function uses a simple first-fit strategy.
   *
   * @param pid The process ID.
   * @param pageNum The page number.
   * @return True if the page was successfully paged in, false otherwise.
   */
  bool PageIn(int pid, int pageNum) {
    auto pidIt = pageTables.find(pid);
    if (pidIt != pageTables.end()) {
      auto pageIt = pidIt->second.find(pageNum);
      if (pageIt != pidIt->second.end() && pageIt->second.resident) {
        return true;
      }
    }

    for (auto &frame : frames) {
      if (!frame.allocated) {
        frame.allocated = true;

        auto &processPages = pageTables[pid];
        processPages[pageNum] = PageTableEntry{true, frame.frameNumber};
        return true;
      }
    }

    // TODO: eviction
    return false;
  }

  /**
   * @brief Frees all frames used by a process and clears its page table entries.
   *
   * @param pid The process ID.
   */
  void FreeAllPagesForProcess(int pid) {
    auto pidIt = pageTables.find(pid);
    if (pidIt == pageTables.end()) {
      return;
    }

    for (auto &entry : pidIt->second) {
      if (entry.second.resident && entry.second.frameNumber >= 0 &&
          entry.second.frameNumber < static_cast<int>(frames.size())) {
        frames[entry.second.frameNumber].allocated = false;
      }
    }

    pidIt->second.clear();
    pageTables.erase(pidIt);
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
