#include <gtest/gtest.h>
#include "config.h"
#include "src/context.h"


namespace ConfigMakeDefault {

    // makeDefault always returns a valid pointer, never nullptr
    TEST(ConfigMakeDefault, ReturnsNonNull) {
        ConfigStruct *cs = makeDefault();
        ASSERT_NE(cs, nullptr);
        delete cs;
    }

    // Default num_cpu is 4
    TEST(ConfigMakeDefault, NumCpuIsDefault) {
        ConfigStruct *cs = makeDefault();
        EXPECT_EQ(cs->num_cpu, 4);
        delete cs;
    }

    // Default scheduler is "rr" — note: config.txt uses "fcfs" instead
    TEST(ConfigMakeDefault, SchedulerIsRr) {
        ConfigStruct *cs = makeDefault();
        EXPECT_EQ(cs->scheduler, "rr");
        delete cs;
    }

    // Default quantum_cycles is 5
    TEST(ConfigMakeDefault, QuantumCyclesIsDefault) {
        ConfigStruct *cs = makeDefault();
        EXPECT_EQ(cs->quantum_cycles, 5);
        delete cs;
    }

    // Default batch_process_freq is 1
    TEST(ConfigMakeDefault, BatchProcessFreqIsDefault) {
        ConfigStruct *cs = makeDefault();
        EXPECT_EQ(cs->batch_process_freq, 1);
        delete cs;
    }

    // Default min_ins is 1000
    TEST(ConfigMakeDefault, MinInsIsDefault) {
        ConfigStruct *cs = makeDefault();
        EXPECT_EQ(cs->min_ins, 1000);
        delete cs;
    }

    // Default max_ins is 2000
    TEST(ConfigMakeDefault, MaxInsIsDefault) {
        ConfigStruct *cs = makeDefault();
        EXPECT_EQ(cs->max_ins, 2000);
        delete cs;
    }

    // Default delay_per_exec is 0
    TEST(ConfigMakeDefault, DelayPerExecIsDefault) {
        ConfigStruct *cs = makeDefault();
        EXPECT_EQ(cs->delay_per_exec, 0);
        delete cs;
    }

} // namespace ConfigMakeDefault


// Expected values from prosched/config.txt:
//   num-cpu 4  |  scheduler fcfs  |  quantum-cycles 5  |  batch-process-freq 1
//   min-ins 1000  |  max-ins 2000  |  delay-per-exec 0

namespace ConfigFromFile {

    // prosched/config.txt is present and readable from the repo root CWD
    TEST(ConfigFromFile, ValidFileReturnsNonNull) {
        ConfigStruct *cs = fromFile();
        ASSERT_NE(cs, nullptr);
        delete cs;
    }

    // "num-cpu 4" in config.txt is parsed to num_cpu
    TEST(ConfigFromFile, ParsesNumCpu) {
        ConfigStruct *cs = fromFile();
        ASSERT_NE(cs, nullptr);
        EXPECT_EQ(cs->num_cpu, 4);
        delete cs;
    }

    // "scheduler fcfs" in config.txt is parsed to scheduler
    TEST(ConfigFromFile, ParsesScheduler) {
        ConfigStruct *cs = fromFile();
        ASSERT_NE(cs, nullptr);
        EXPECT_EQ(cs->scheduler, "fcfs");
        delete cs;
    }

    // "quantum-cycles 5" is parsed to quantum_cycles
    TEST(ConfigFromFile, ParsesQuantumCycles) {
        ConfigStruct *cs = fromFile();
        ASSERT_NE(cs, nullptr);
        EXPECT_EQ(cs->quantum_cycles, 5);
        delete cs;
    }

    // "batch-process-freq 1" is parsed to batch_process_freq
    TEST(ConfigFromFile, ParsesBatchProcessFreq) {
        ConfigStruct *cs = fromFile();
        ASSERT_NE(cs, nullptr);
        EXPECT_EQ(cs->batch_process_freq, 1);
        delete cs;
    }

    // "min-ins 1000" is parsed to min_ins
    TEST(ConfigFromFile, ParsesMinIns) {
        ConfigStruct *cs = fromFile();
        ASSERT_NE(cs, nullptr);
        EXPECT_EQ(cs->min_ins, 1000);
        delete cs;
    }

    // "max-ins 2000" is parsed to max_ins
    TEST(ConfigFromFile, ParsesMaxIns) {
        ConfigStruct *cs = fromFile();
        ASSERT_NE(cs, nullptr);
        EXPECT_EQ(cs->max_ins, 2000);
        delete cs;
    }

    // "delay-per-exec 0" is parsed to delay_per_exec
    TEST(ConfigFromFile, ParsesDelayPerExec) {
        ConfigStruct *cs = fromFile();
        ASSERT_NE(cs, nullptr);
        EXPECT_EQ(cs->delay_per_exec, 0);
        delete cs;
    }

} // namespace ConfigFromFile


namespace ConfigBuildConfig {

    // num_cpu field transfers to AlgoContext.numCpu
    TEST(ConfigBuildConfig, MapsNumCpu) {
        ConfigStruct cs{.num_cpu = 8, .scheduler = "fcfs", .quantum_cycles = 10,
                        .batch_process_freq = 2, .min_ins = 500, .max_ins = 1000, .delay_per_exec = 1};
        AlgoContext ctx = AlgoContext::buildConfig(&cs);
        EXPECT_EQ(ctx.numCpu, 8);
    }

    // "fcfs" string becomes SchedulerType::FCFS enum
    TEST(ConfigBuildConfig, MapsSchedulerFcfs) {
        ConfigStruct cs{.num_cpu = 4, .scheduler = "fcfs", .quantum_cycles = 5,
                        .batch_process_freq = 1, .min_ins = 1000, .max_ins = 2000, .delay_per_exec = 0};
        AlgoContext ctx = AlgoContext::buildConfig(&cs);
        EXPECT_EQ(ctx.schedulerType, SchedulerType::FCFS);
    }

    // "rr" string becomes SchedulerType::RR enum
    TEST(ConfigBuildConfig, MapsSchedulerRr) {
        ConfigStruct cs{.num_cpu = 4, .scheduler = "rr", .quantum_cycles = 5,
                        .batch_process_freq = 1, .min_ins = 1000, .max_ins = 2000, .delay_per_exec = 0};
        AlgoContext ctx = AlgoContext::buildConfig(&cs);
        EXPECT_EQ(ctx.schedulerType, SchedulerType::RR);
    }

    // Unrecognized scheduler string becomes SchedulerType::UNKNOWN
    TEST(ConfigBuildConfig, UnknownSchedulerMapsToUnknown) {
        ConfigStruct cs{.num_cpu = 4, .scheduler = "sjf", .quantum_cycles = 5,
                        .batch_process_freq = 1, .min_ins = 1000, .max_ins = 2000, .delay_per_exec = 0};
        AlgoContext ctx = AlgoContext::buildConfig(&cs);
        EXPECT_EQ(ctx.schedulerType, SchedulerType::UNKNOWN);
    }

    // batch_process_freq transfers to batchProcessFreq
    TEST(ConfigBuildConfig, MapsBatchProcessFreq) {
        ConfigStruct cs{.num_cpu = 4, .scheduler = "fcfs", .quantum_cycles = 5,
                        .batch_process_freq = 3, .min_ins = 1000, .max_ins = 2000, .delay_per_exec = 0};
        AlgoContext ctx = AlgoContext::buildConfig(&cs);
        EXPECT_EQ(ctx.batchProcessFreq, 3);
    }

    // min_ins and max_ins both transfer correctly
    TEST(ConfigBuildConfig, MapsMinAndMaxIns) {
        ConfigStruct cs{.num_cpu = 4, .scheduler = "fcfs", .quantum_cycles = 5,
                        .batch_process_freq = 1, .min_ins = 200, .max_ins = 800, .delay_per_exec = 0};
        AlgoContext ctx = AlgoContext::buildConfig(&cs);
        EXPECT_EQ(ctx.minIns, 200);
        EXPECT_EQ(ctx.maxIns, 800);
    }

    // quantum_cycles transfers to quantumCycles
    TEST(ConfigBuildConfig, MapsQuantumCycles) {
        ConfigStruct cs{.num_cpu = 4, .scheduler = "fcfs", .quantum_cycles = 7,
                        .batch_process_freq = 1, .min_ins = 1000, .max_ins = 2000, .delay_per_exec = 0};
        AlgoContext ctx = AlgoContext::buildConfig(&cs);
        EXPECT_EQ(ctx.quantumCycles, 7);
    }

    // delay_per_exec transfers to delayPerExec
    TEST(ConfigBuildConfig, MapsDelayPerExec) {
        ConfigStruct cs{.num_cpu = 4, .scheduler = "fcfs", .quantum_cycles = 5,
                        .batch_process_freq = 1, .min_ins = 1000, .max_ins = 2000, .delay_per_exec = 5};
        AlgoContext ctx = AlgoContext::buildConfig(&cs);
        EXPECT_EQ(ctx.delayPerExec, 5);
    }

} // namespace ConfigBuildConfig
