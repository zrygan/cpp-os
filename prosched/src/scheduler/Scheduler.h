#pragma once

#include <stdio.h>
#include <vector>
#include <queue>
#include <mutex>
#include <thread>

#include "config.h"
#include "process/Process.h"
#include "../context.h"

class Scheduler {
public:
    Scheduler(AlgoContext ctx);

    // these two have input?
    void Start(
        // dfn some inputs (workers)
    );
    void Stop();

    // for AddProcess return the process itself on success.
    // none if not success.
    Process AddProcess(Process *p);
    void PrintProcesses();

private:
    AlgoContext ctx;
    std::vector<Process> processes;
    std::queue<Process> processQueue;
    std::vector<std::thread> workerThreads;
    std::mutex schedulerMutex;
    bool running = false;

    void SchedulerLoop();
};