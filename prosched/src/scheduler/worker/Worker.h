#pragma once

#include <stdio.h>
#include <thread>
#include <mutex>

#include "config.h"
#include "src/scheduler/process/Process.h"
#include "src/context.h"

class Worker {
public:
    Worker(int coreNum);

    void Start();
    void Stop();
    void AssignProcess(Process* p);

private:
    AlgoContext ctx;
    int coreNum;
    Process* currentProcess = nullptr;
    std::thread workerThread;
    std::mutex workerMutex;
    bool running = false;

    void ThreadTask();

};