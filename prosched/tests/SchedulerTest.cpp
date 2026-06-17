#include <gtest/gtest.h>
#include "scheduler/Scheduler.h"
#include "scheduler/process/Process.h"
#include "config.h"
#include "context.h"

static AlgoContext makeTestCtx() {
    ConfigStruct *cs = makeDefault();
    cs->scheduler = "fcfs";
    AlgoContext ctx = AlgoContext::buildConfig(cs);
    delete cs;
    return ctx;
}

// AddProcess should return the process that was registered
TEST(SchedulerAddProcess, ReturnsSameProcess) {
    Scheduler scheduler(makeTestCtx());
    Process p("test_process", 1, 0);

    Process result = scheduler.AddProcess(&p);

    EXPECT_EQ(result.GetPID(), p.GetPID());
    EXPECT_EQ(result.GetName(), p.GetName());
}

// Adding multiple processes should each return their own identity
TEST(SchedulerAddProcess, MultipleProcessesReturnCorrectly) {
    Scheduler scheduler(makeTestCtx());
    Process p1("proc_alpha", 1, 0);
    Process p2("proc_beta",  2, 1);

    Process r1 = scheduler.AddProcess(&p1);
    Process r2 = scheduler.AddProcess(&p2);

    EXPECT_EQ(r1.GetPID(),  1);
    EXPECT_EQ(r1.GetName(), "proc_alpha");
    EXPECT_EQ(r2.GetPID(),  2);
    EXPECT_EQ(r2.GetName(), "proc_beta");
}

// Process with an empty name string should still be accepted
TEST(SchedulerAddProcess, EmptyProcessName) {
    Scheduler scheduler(makeTestCtx());
    Process p("", 1, 0);

    Process result = scheduler.AddProcess(&p);

    EXPECT_EQ(result.GetPID(),  1);
    EXPECT_EQ(result.GetName(), "");
}

// PID 0 is a valid boundary value
TEST(SchedulerAddProcess, PIDZero) {
    Scheduler scheduler(makeTestCtx());
    Process p("pid_zero", 0, 0);

    Process result = scheduler.AddProcess(&p);

    EXPECT_EQ(result.GetPID(),  0);
    EXPECT_EQ(result.GetName(), "pid_zero");
}

// Adding the same process pointer twice — second call should still return
// a process with the same identity (scheduler does not crash or corrupt)
TEST(SchedulerAddProcess, DuplicatePID) {
    Scheduler scheduler(makeTestCtx());
    Process p("dup", 5, 0);

    Process r1 = scheduler.AddProcess(&p);
    Process r2 = scheduler.AddProcess(&p);

    EXPECT_EQ(r1.GetPID(), r2.GetPID());
    EXPECT_EQ(r1.GetName(), r2.GetName());
}

// Arrival tick at INT_MAX should not cause overflow or crash
TEST(SchedulerAddProcess, LargeArrivalTick) {
    Scheduler scheduler(makeTestCtx());
    Process p("late_proc", 1, INT_MAX);

    Process result = scheduler.AddProcess(&p);

    EXPECT_EQ(result.GetPID(),  1);
    EXPECT_EQ(result.GetName(), "late_proc");
}

// Adding more processes than num_cpu (4 in default config) should not
// drop or corrupt any of them
TEST(SchedulerAddProcess, MoreProcessesThanCores) {
    Scheduler scheduler(makeTestCtx());
    const int count = 10;
    std::vector<Process> procs;
    procs.reserve(count);
    for (int i = 0; i < count; i++)
        procs.emplace_back("p" + std::to_string(i), i, i);

    for (int i = 0; i < count; i++) {
        Process result = scheduler.AddProcess(&procs[i]);
        EXPECT_EQ(result.GetPID(),  i);
        EXPECT_EQ(result.GetName(), "p" + std::to_string(i));
    }
}

// A process with a later arrival tick is submitted before one with an earlier
// tick. Both should still be accepted and return their own correct identity —
// the scheduler must not reject a process just because it was enqueued out of
// chronological order.
TEST(SchedulerAddProcess, OutOfOrderArrivalTick) {
    Scheduler scheduler(makeTestCtx());
    Process late ("late_proc",  1, 100); // logically arrives at tick 100
    Process early("early_proc", 2, 0);   // logically arrives at tick 0

    // submit the late-arriving one first
    Process r_late  = scheduler.AddProcess(&late);
    Process r_early = scheduler.AddProcess(&early);

    EXPECT_EQ(r_late.GetPID(),   1);
    EXPECT_EQ(r_late.GetName(),  "late_proc");
    EXPECT_EQ(r_early.GetPID(),  2);
    EXPECT_EQ(r_early.GetName(), "early_proc");
}

// Passing nullptr should be treated as an invalid process.
// AddProcess must return a sentinel Process with PID -1 instead of
// crashing or dereferencing the null pointer.
TEST(SchedulerAddProcess, NullProcessReturnsNone) {
    Scheduler scheduler(makeTestCtx());

    Process result = scheduler.AddProcess(nullptr);

    EXPECT_EQ(result.GetPID(), -1);
}
