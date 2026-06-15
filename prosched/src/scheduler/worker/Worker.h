#pragma once

#include <bits/stdc++.h>
#include <stdio.h>
#include <thread>

#include "../config.h"
#include "Process.h"
#include "../context.h"

class Worker {
public:
    Worker(int coreNum);

    void start();
    void stop();
    void assignProcess(Process* p);

private:
    AlgoContext ctx;
    int coreNum;
    Process* currentProcess = nullptr;
    std::thread workerThread;
    bool running = false;

    void ThreadTask();
    void WriteLogs();

};