#pragma once

namespace prosched {
    constexpr int kMaxProcesses = 10;            // limit for scheduler-test
    constexpr int kNumPrintInstructions = 100;  // default prints per process
    constexpr int kTickDurationMs = 10;         // duration of a single tick in milliseconds
}