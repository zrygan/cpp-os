#pragma once

#include <bits/stdc++.h>
#include <stdio.h>
#include <vector>
#include <queue>

#include "../config.h"
#include "Process.h"
#include "../context.h"

class Scheduler {
public:
    Scheduler(ConfigStruct *config);

    void Start();
    void Stop();

    void AddProcess(Process *p);
    void PrintProcesses();

private:
    AlgoContext ctx;
    std::vector<Process> processes;
    std::queue<Process> processQueue;
    std::vector<std::thread> workerThreads;
    bool running = false;

    void SchedulerLoop();
};