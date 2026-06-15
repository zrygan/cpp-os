#include <bits/stdc++.h>
#include <queue>
#include <stdio.h>
#include <vector>

#include "../config.h"
#include "Process.cpp"
#include "Process.h"

using namespace std;

extern ConfigStruct globalConfig;

extern int cpuCycles;
extern bool schedulerRunning;

#include <cstdlib>

void fcfsScheduler(ConfigStruct *config, bool isRunning) {
  std::vector<Process *> processVec;
  int cpuCycles = 0;
  int nextID = 0;

  while (isRunning) {
    cpuCycles++;

    Process *p = generateProcess(config, nextID, cpuCycles);
    processVec.push_back(p);
  }
}