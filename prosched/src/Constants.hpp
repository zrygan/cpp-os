#pragma once

namespace prosched {
    constexpr int kMaxProcesses = 10;           // limit for scheduler-test
    constexpr int kNumPrintInstructions = 100;  // default prints per process
    constexpr int kTickDurationMs = 0;         // duration of a single tick in milliseconds
    constexpr long kMinProcessMemoryBytes = 64;    // 2^6
    constexpr long kMaxProcessMemoryBytes = 65536; // 2^16
}