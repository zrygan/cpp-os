#include <gtest/gtest.h>
#include "scheduler/worker/Worker.h"
#include "scheduler/process/Process.h"

static AlgoContext makeTestCtx() {
    ConfigStruct *cs = makeDefault();
    cs->scheduler = "fcfs";
    AlgoContext ctx = AlgoContext::buildConfig(cs);
    delete cs;
    return ctx;
}

// Assigning a process while the worker is stopped should still succeed
// Assigning a valid process should return the same pointer that was passed in
TEST(WorkerAssignProcess, ReturnsAssignedProcess) {
    Worker worker(0, makeTestCtx());
    Process p("proc", 1, 0);

    Process *result = worker.AssignProcess(&p);

    EXPECT_EQ(result, &p);
}

// Assigning nullptr should return nullptr (explicit "no process" assignment)
TEST(WorkerAssignProcess, AssignNullptrReturnsNull) {
    Worker worker(0, makeTestCtx());

    Process *result = worker.AssignProcess(nullptr);

    EXPECT_EQ(result, nullptr);
}

// Reassigning to a different process — returns the new process, not the old one
TEST(WorkerAssignProcess, ReassignReturnsNewProcess) {
    Worker worker(0, makeTestCtx());
    Process p1("first",  1, 0);
    Process p2("second", 2, 0);

    worker.AssignProcess(&p1);
    Process *result = worker.AssignProcess(&p2);

    EXPECT_EQ(result, &p2);
}

// Assigning the same pointer twice should return it both times without issue
TEST(WorkerAssignProcess, AssignSameProcessTwice) {
    Worker worker(0, makeTestCtx());
    Process p("proc", 1, 0);

    Process *r1 = worker.AssignProcess(&p);
    Process *r2 = worker.AssignProcess(&p);

    EXPECT_EQ(r1, &p);
    EXPECT_EQ(r2, &p);
}

// Assigning nullptr after a valid process (unassign) — should return nullptr
TEST(WorkerAssignProcess, UnassignAfterValidProcess) {
    Worker worker(0, makeTestCtx());
    Process p("proc", 1, 0);

    worker.AssignProcess(&p);
    Process *result = worker.AssignProcess(nullptr);

    EXPECT_EQ(result, nullptr);
}

// Worker on core 0 vs high core numbers — AssignProcess should behave the same
TEST(WorkerAssignProcess, WorksAcrossCoreBoundaries) {
    Worker w_zero(0);
    Worker w_high(INT_MAX);
    Process p("proc", 1, 0);

    EXPECT_EQ(w_zero.AssignProcess(&p), &p);
    EXPECT_EQ(w_high.AssignProcess(&p), &p);
}


// Assigning a process while the worker is running should still succeed
TEST(WorkerAssignProcess, AssignWhileRunning) {
    Worker worker(0, makeTestCtx());
    Process p("proc", 1, 0);

    worker.Start();
    Process *result = worker.AssignProcess(&p);
    worker.Stop();

    EXPECT_EQ(result, &p);
}
