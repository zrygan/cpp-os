#include "scheduler/Scheduler.h"
#include "config.h"
#include "context.h"
#include "scheduler/process/Process.h"
#include <chrono>
#include <gtest/gtest.h>
#include <thread>

static AlgoContext makeTestCtx() {
  ConfigStruct *cs = makeDefault();
  cs->scheduler = "fcfs";
  AlgoContext ctx = AlgoContext::buildConfig(cs);
  delete cs;
  return ctx;
}

namespace SchedulerAddProcess {
// Returns the same pointer that was passed in
TEST(SchedulerAddProcess, ReturnsSameProcess) {
  prosched::Scheduler scheduler(makeTestCtx());
  prosched::Process p("test_process", 1, 0);

  prosched::Process *result = scheduler.AddProcess(&p);

  ASSERT_NE(result, nullptr);
  EXPECT_EQ(result->GetPID(), p.GetPID());
  EXPECT_EQ(result->GetName(), p.GetName());
}

// Adding multiple processes should each return their own identity
TEST(SchedulerAddProcess, MultipleProcessesReturnCorrectly) {
  prosched::Scheduler scheduler(makeTestCtx());
  prosched::Process p1("proc_alpha", 1, 0);
  prosched::Process p2("proc_beta", 2, 1);

  prosched::Process *r1 = scheduler.AddProcess(&p1);
  prosched::Process *r2 = scheduler.AddProcess(&p2);

  ASSERT_NE(r1, nullptr);
  ASSERT_NE(r2, nullptr);
  EXPECT_EQ(r1->GetPID(), 1);
  EXPECT_EQ(r1->GetName(), "proc_alpha");
  EXPECT_EQ(r2->GetPID(), 2);
  EXPECT_EQ(r2->GetName(), "proc_beta");
}

// Empty process name is still accepted
TEST(SchedulerAddProcess, EmptyProcessName) {
  prosched::Scheduler scheduler(makeTestCtx());
  prosched::Process p("", 1, 0);

  prosched::Process *result = scheduler.AddProcess(&p);

  ASSERT_NE(result, nullptr);
  EXPECT_EQ(result->GetName(), "");
}

// PID 0 is a valid boundary value
TEST(SchedulerAddProcess, PIDZero) {
  prosched::Scheduler scheduler(makeTestCtx());
  prosched::Process p("pid_zero", 0, 0);

  prosched::Process *result = scheduler.AddProcess(&p);

  ASSERT_NE(result, nullptr);
  EXPECT_EQ(result->GetPID(), 0);
}

// Same pointer added twice — scheduler must not crash or corrupt
TEST(SchedulerAddProcess, DuplicatePID) {
  prosched::Scheduler scheduler(makeTestCtx());
  prosched::Process p("dup", 5, 0);

  prosched::Process *r1 = scheduler.AddProcess(&p);
  prosched::Process *r2 = scheduler.AddProcess(&p);

  ASSERT_NE(r1, nullptr);
  ASSERT_NE(r2, nullptr);
  EXPECT_EQ(r1->GetPID(), r2->GetPID());
  EXPECT_EQ(r1->GetName(), r2->GetName());
}

// INT_MAX arrival tick must not overflow or crash
TEST(SchedulerAddProcess, LargeArrivalTick) {
  prosched::Scheduler scheduler(makeTestCtx());
  prosched::Process p("late_proc", 1, INT_MAX);

  prosched::Process *result = scheduler.AddProcess(&p);

  ASSERT_NE(result, nullptr);
  EXPECT_EQ(result->GetPID(), 1);
  EXPECT_EQ(result->GetName(), "late_proc");
}

// Adding more processes than num_cpu must not drop any
TEST(SchedulerAddProcess, MoreProcessesThanCores) {
  prosched::Scheduler scheduler(makeTestCtx());
  const int count = 10;
  std::vector<prosched::Process> procs;
  procs.reserve(count);
  for (int i = 0; i < count; i++)
    procs.emplace_back("p" + std::to_string(i), i, i);

  for (int i = 0; i < count; i++) {
    prosched::Process *result = scheduler.AddProcess(&procs[i]);
    ASSERT_NE(result, nullptr) << "Failed at index " << i;
    EXPECT_EQ(result->GetPID(), i);
    EXPECT_EQ(result->GetName(), "p" + std::to_string(i));
  }
}

// Out-of-chronological-order arrival ticks must both be accepted
TEST(SchedulerAddProcess, OutOfOrderArrivalTick) {
  prosched::Scheduler scheduler(makeTestCtx());
  prosched::Process late("late_proc", 1, 100);
  prosched::Process early("early_proc", 2, 0);

  prosched::Process *r_late = scheduler.AddProcess(&late);
  prosched::Process *r_early = scheduler.AddProcess(&early);

  ASSERT_NE(r_late, nullptr);
  ASSERT_NE(r_early, nullptr);
  EXPECT_EQ(r_late->GetName(), "late_proc");
  EXPECT_EQ(r_early->GetName(), "early_proc");
}

// nullptr input must return nullptr and not crash
TEST(SchedulerAddProcess, NullProcessReturnsNull) {
  prosched::Scheduler scheduler(makeTestCtx());

  prosched::Process *result = scheduler.AddProcess(nullptr);

  EXPECT_EQ(result, nullptr);
}

} // namespace SchedulerAddProcess

namespace SchedulerStartStop {

TEST(SchedulerStartStop, IsRunningFalseOnConstruction) {
  prosched::Scheduler scheduler(makeTestCtx());
  EXPECT_FALSE(scheduler.IsRunning());
}

// Start() returns true on a fresh scheduler
TEST(SchedulerStartStop, StartReturnsTrueOnSuccess) {
  prosched::Scheduler scheduler(makeTestCtx());

  bool result = scheduler.Start();
  scheduler.Stop();

  EXPECT_TRUE(result);
}

TEST(SchedulerStartStop, IsRunningTrueAfterStart) {
  prosched::Scheduler scheduler(makeTestCtx());

  scheduler.Start();
  EXPECT_TRUE(scheduler.IsRunning());

  scheduler.Stop();
}

// Start() returns false and leaves state unchanged when already running
TEST(SchedulerStartStop, StartReturnsFalseIfAlreadyRunning) {
  prosched::Scheduler scheduler(makeTestCtx());

  scheduler.Start();
  bool result = scheduler.Start(); // second call

  EXPECT_FALSE(result);
  EXPECT_TRUE(scheduler.IsRunning()); // still running

  scheduler.Stop();
}

TEST(SchedulerStartStop, IsRunningFalseAfterStop) {
  prosched::Scheduler scheduler(makeTestCtx());

  scheduler.Start();
  scheduler.Stop();

  EXPECT_FALSE(scheduler.IsRunning());
}

// Stop() on a scheduler that never started must not crash
TEST(SchedulerStartStop, StopWithoutStartDoesNotCrash) {
  prosched::Scheduler scheduler(makeTestCtx());
  EXPECT_NO_THROW(scheduler.Stop());
  EXPECT_FALSE(scheduler.IsRunning());
}

// Stop() while the SchedulerLoop is actively ticking must not deadlock
TEST(SchedulerStartStop, StopWhileSchedulingDoesNotDeadlock) {
  prosched::Scheduler scheduler(makeTestCtx());

  scheduler.Start();
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  scheduler.Stop();

  EXPECT_FALSE(scheduler.IsRunning());
}

// Stop() while workers are mid-execution must still clean up cleanly
TEST(SchedulerStartStop, StopWhileWorkersExecutingDoesNotDeadlock) {
  prosched::Scheduler scheduler(makeTestCtx());

  // Pre-load processes before starting so workers get busy immediately
  for (int i = 1; i <= 4; i++) {
    prosched::Process *p =
        new prosched::Process("pre_" + std::to_string(i), i, 0);
    for (int j = 0; j < 20; j++)
      p->AddInstruction("PRINT(\"tick\")");
    scheduler.AddProcess(p);
  }

  scheduler.Start();
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  scheduler.Stop();

  EXPECT_FALSE(scheduler.IsRunning());
}

// Start/Stop can cycle multiple times without deadlock or corrupted state
TEST(SchedulerStartStop, MultipleRestartCycles) {
  prosched::Scheduler scheduler(makeTestCtx());

  for (int i = 0; i < 3; i++) {
    bool started = scheduler.Start();
    EXPECT_TRUE(started) << "Start failed on cycle " << i;
    EXPECT_TRUE(scheduler.IsRunning());

    scheduler.Stop();
    EXPECT_FALSE(scheduler.IsRunning());
  }
}

} // namespace SchedulerStartStop

namespace SchedulerPrintProcesses {

// Printing with no processes registered must not crash
TEST(SchedulerPrintProcesses, EmptyListDoesNotCrash) {
  prosched::Scheduler scheduler(makeTestCtx());
  EXPECT_NO_THROW(scheduler.PrintProcesses());
}

// Printing after AddProcess must not crash
TEST(SchedulerPrintProcesses, PrintAfterAddDoesNotCrash) {
  prosched::Scheduler scheduler(makeTestCtx());
  prosched::Process p("print_test", 1, 0);
  scheduler.AddProcess(&p);

  EXPECT_NO_THROW(scheduler.PrintProcesses());
}

// PrintProcesses while the scheduler is running must not deadlock
TEST(SchedulerPrintProcesses, PrintWhileRunningDoesNotDeadlock) {
  prosched::Scheduler scheduler(makeTestCtx());

  scheduler.Start();
  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  EXPECT_NO_THROW(scheduler.PrintProcesses());

  scheduler.Stop();
}

} // namespace SchedulerPrintProcesses
