#pragma once

#include <stdio.h>
#include <vector>
#include <string>

#include "config.h"

struct AlgoContext {
    std::string schedulerType;
    int numCpu;
    int batchProcessFreq;
    int minIns;
    int maxIns;
    int delayPerExec;
    // for rr
    int quantumCycles;

    static AlgoContext buildConfig(ConfigStruct *config){
        AlgoContext ctx;
        ctx.schedulerType = config->scheduler;
        ctx.numCpu = config->num_cpu;
        ctx.batchProcessFreq = config->batch_process_freq;
        ctx.minIns = config->min_ins;
        ctx.maxIns = config->max_ins;
        ctx.delayPerExec = config->delay_per_exec;
        ctx.quantumCycles = config->quantum_cycles;
        return ctx;
    }
};