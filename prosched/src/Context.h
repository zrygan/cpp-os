#pragma once

#include <stdio.h>
#include <string>
#include <vector>

#include "Config.h"

enum class SchedulerType { FCFS, RR, UNKNOWN };

struct AlgoContext {
  SchedulerType schedulerType = SchedulerType::UNKNOWN;
  int numCpu;
  int batchProcessFreq;
  int minIns;
  int maxIns;
  int delayPerExec;
  // for rr
  int quantumCycles;

  static AlgoContext buildConfig(ConfigStruct *config) {
    AlgoContext ctx;

    std::string sched = config->scheduler;
    if (sched == "fcfs")
      ctx.schedulerType = SchedulerType::FCFS;
    else if (sched == "rr")
      ctx.schedulerType = SchedulerType::RR;
    else
      ctx.schedulerType = SchedulerType::UNKNOWN;

    ctx.numCpu = config->num_cpu;
    ctx.batchProcessFreq = config->batch_process_freq;
    ctx.minIns = config->min_ins;
    ctx.maxIns = config->max_ins;
    ctx.delayPerExec = config->delay_per_exec;
    ctx.quantumCycles = config->quantum_cycles;
    return ctx;
  }
};