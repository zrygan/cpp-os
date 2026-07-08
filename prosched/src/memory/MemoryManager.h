#pragma once

#include <string>
#include <cstddef>
#include <vector>
#include <sstream>

namespace prosched {

class MemoryManager {
public:
    struct Block {
        size_t start;
        size_t end;
        bool isFree;
        int pid;
    };

    /**
     * @brief Constructs a MemoryManager with the specified total memory and process size.
     * 
     * @param totalMemory The total memory available for allocation.
     * @param procSize The size of each process to be allocated.
     */
    MemoryManager(size_t totalMemory, size_t procSize)
        : totalMemory(totalMemory), procSize(procSize) {
        if (totalMemory > 0) {
            blocks.push_back({0, totalMemory, true, -1});
        }
    }

    /**
     * @brief Allocates memory for a process with the given PID.
     * 
     * @param pid The process ID for which to allocate memory.
     * @return true if allocation was successful, false otherwise.
     */
    bool Allocate(int pid) {
        if (procSize == 0 || totalMemory < procSize) {
            return false;
        }

        // Avoid duplicate allocation for the same pid
        for (const auto& block : blocks) {
            if (!block.isFree && block.pid == pid) {
                return false;
            }
        }

        for (auto it = blocks.begin(); it != blocks.end(); ++it) {
            if (it->isFree) {
                size_t blockSize = it->end - it->start;
                if (blockSize >= procSize) {
                    if (blockSize > procSize) {
                        // Split the block
                        Block freeRemainder;
                        freeRemainder.start = it->start + procSize;
                        freeRemainder.end = it->end;
                        freeRemainder.isFree = true;
                        freeRemainder.pid = -1;

                        it->end = it->start + procSize;
                        it->isFree = false;
                        it->pid = pid;

                        blocks.insert(it + 1, freeRemainder);
                    } else {
                        // Exact size match
                        it->isFree = false;
                        it->pid = pid;
                    }
                    return true;
                }
            }
        }
        return false;
    }

    /**
     * @brief Frees the memory allocated for a process with the given PID.
     * 
     * @param pid The process ID for which to free memory.
     */
    void Free(int pid) {
        bool freedAny = false;
        for (auto& block : blocks) {
            if (!block.isFree && block.pid == pid) {
                block.isFree = true;
                block.pid = -1;
                freedAny = true;
            }
        }

        if (freedAny) {
            Coalesce();
        }
    }

    /**
     * @brief Returns the total number of processes currently allocated in memory.
     * 
     * @return The total number of processes currently allocated in memory.
     */
    int GetProcessCount() const {
        int count = 0;
        for (const auto& block : blocks) {
            if (!block.isFree) {
                count++;
            }
        }
        return count;
    }

    /**
     * @brief Returns the amount of external fragmentation in the memory.
     * 
     * @return The amount of external fragmentation in the memory.
     */
    size_t GetExternalFragmentation() const {
        size_t fragmentation = 0;
        for (const auto& block : blocks) {
            if (block.isFree && (block.end - block.start) < procSize) {
                fragmentation += (block.end - block.start);
            }
        }
        return fragmentation;
    }

    /**
     * @brief Returns an ASCII snapshot of the memory state at the given timestamp.
     * 
     * @param timestamp The timestamp for the snapshot.
     * @param quantumCycle The quantum cycle for the snapshot.
     * @return The ASCII snapshot of the memory state.
     */
    std::string GetAsciiSnapshot(long timestamp, int quantumCycle) const {
        std::stringstream ss;
        ss << "Timestamp: " << timestamp << " | Quantum Cycle: " << quantumCycle << "\n";
        ss << "Memory Layout:\n";
        for (const auto& block : blocks) {
            ss << "[" << block.start << " - " << block.end << "] ";
            if (block.isFree) {
                ss << "Free (" << (block.end - block.start) << " bytes)\n";
            } else {
                ss << "PID: " << block.pid << " (" << (block.end - block.start) << " bytes)\n";
            }
        }
        return ss.str();
    }

    // Exposed getter for testing/inspection
    const std::vector<Block>& GetBlocks() const {
        return blocks;
    }

private:
    size_t totalMemory;
    size_t procSize;
    std::vector<Block> blocks;

    /**
     * @brief Coalesces adjacent free memory blocks.
     */
    void Coalesce() {
        if (blocks.empty()) {
            return;
        }

        std::vector<Block> merged;
        merged.push_back(blocks[0]);

        for (size_t i = 1; i < blocks.size(); ++i) {
            if (merged.back().isFree && blocks[i].isFree) {
                merged.back().end = blocks[i].end;
            } else {
                merged.push_back(blocks[i]);
            }
        }

        blocks = std::move(merged);
    }
};

} // namespace prosched
