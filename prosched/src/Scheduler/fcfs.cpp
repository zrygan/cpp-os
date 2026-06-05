#include <stdio.h>
#include<vector>
#include <queue>
#include <bits/stdc++.h>

#include "../config.h"
#include "Process.h"
#include "Process.cpp"

using namespace std;

extern ConfigStruct globalConfig;

extern int cpuCycles;
extern bool schedulerRunning;

#include <cstdlib>

void fcfsScheduler(ConfigStruct* config, bool isRunning){
    std::vector <Process*> processVec;
    int cpuCycles = 0;
    int nextID = 0;

    while (isRunning){ 
        cpuCycles++;

        Process* p = generateProcess(config, nextID, cpuCycles);
        processVec.push_back(p);
    }
}