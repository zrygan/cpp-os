#include "scheduler/Scheduler.h"
#include "Config.h"
#include "Context.h"
#include "scheduler/process/Process.h"
#include <chrono>
#include <gtest/gtest.h>
#include <sstream>
#include <thread>

static void AddRaw(prosched::Process &p, const std::string &src) {
  prosched::Interpreter interp;
  auto stmts = interp.parse(src);
  for (auto &s : stmts)
    p.AddInstruction(s);
}

static AlgoContext makeTestCtx() {
  ConfigStruct *cs = makeDefault();
  cs->scheduler = "fcfs";
  AlgoContext ctx = AlgoContext::buildConfig(cs);
  delete cs;
  return ctx;
}

// Small-instruction context with suppressed auto-generation (batch_freq=1M).
// algo: "fcfs" or "rr"; quantum only relevant for rr.
static AlgoContext makeSmallCtx(const std::string &algo, int num_cpu = 1,
                                int quantum = 3) {
  ConfigStruct *cs = makeDefault();
  cs->scheduler = algo;
  cs->num_cpu = num_cpu;
  cs->rr_quantum_cycles = quantum;
  cs->batch_process_freq = 1000000;
  cs->min_ins = 1;
  cs->max_ins = 5;
  AlgoContext ctx = AlgoContext::buildConfig(cs);
  delete cs;
  return ctx;
}

namespace SchedulerAddProcess {

// AddProcess returns the same pointer that was passed in
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

// AddProcess acquires schedulerMutex; SchedulerLoop also acquires it for
// process generation and FCFS — must return the same pointer even under
// contention
TEST(SchedulerAddProcess, AddWhileRunningReturnsSamePointer) {
  prosched::Scheduler scheduler(makeTestCtx());
  scheduler.Start();

  prosched::Process p("live_add", 1, 0);
  AddRaw(p, "PRINT(\"hi\")");

  prosched::Process *result = scheduler.AddProcess(&p);
  EXPECT_EQ(result, &p);

  scheduler.Stop();
}

} // namespace SchedulerAddProcess

namespace SchedulerStartStop {

// Scheduler thread is not started on construction
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

// IsRunning reflects the scheduler being active after Start
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

// IsRunning drops to false once Stop completes
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
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
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
      AddRaw(*p, "PRINT(\"tick\")");
    scheduler.AddProcess(p);
  }

  scheduler.Start();
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
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
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  EXPECT_NO_THROW(scheduler.PrintProcesses());

  scheduler.Stop();
}

// PrintProcesses holds schedulerMutex then acquires workerMutex via
// GetCurrentProcess — two concurrent callers must not produce lock-order
// inversion; scheduler must still be running correctly after both complete
TEST(SchedulerPrintProcesses, ConcurrentPrintLeavesSchedulerRunning) {
  prosched::Scheduler scheduler(makeTestCtx());
  scheduler.Start();
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  std::thread t1([&] { scheduler.PrintProcesses(); });
  std::thread t2([&] { scheduler.PrintProcesses(); });

  t1.join();
  t2.join();

  EXPECT_TRUE(scheduler.IsRunning());

  scheduler.Stop();
}

} // namespace SchedulerPrintProcesses

namespace SchedulerGenerateProcess {

// generateProcess() is public and does NOT increment nextPID — that happens
// only inside SchedulerLoop. A fresh Scheduler has nextPID == 1, so the first
// call always produces name "process1" regardless of the pid argument.

// generateProcess always returns a valid heap-allocated Process
TEST(SchedulerGenerateProcess, ReturnsNonNull) {
  prosched::Scheduler scheduler(makeTestCtx());
  AlgoContext ctx = makeTestCtx();
  prosched::Process *p = scheduler.generateProcess(&ctx, 1, 0);
  ASSERT_NE(p, nullptr);
  delete p;
}

// Name uses nextPID (starts at 1), not the pid parameter
TEST(SchedulerGenerateProcess, NameFollowsProcessNPattern) {
  prosched::Scheduler scheduler(makeTestCtx());
  AlgoContext ctx = makeTestCtx();
  prosched::Process *p = scheduler.generateProcess(&ctx, 1, 0);
  ASSERT_NE(p, nullptr);
  EXPECT_EQ(p->GetName(), "process1");
  delete p;
}

// Instruction count falls within the configured min_ins/max_ins range
TEST(SchedulerGenerateProcess, InstructionCountInConfigRange) {
  prosched::Scheduler scheduler(makeTestCtx());
  AlgoContext ctx = makeTestCtx();
  prosched::Process *p = scheduler.generateProcess(&ctx, 1, 0);
  ASSERT_NE(p, nullptr);
  int total = p->GetTotalInstructions();
  EXPECT_GE(total, ctx.min_ins);
  EXPECT_LE(total, ctx.max_ins);
  delete p;
}

// Processes are flagged for cleanup by the Scheduler destructor
TEST(SchedulerGenerateProcess, IsOwnedByScheduler) {
  prosched::Scheduler scheduler(makeTestCtx());
  AlgoContext ctx = makeTestCtx();
  prosched::Process *p = scheduler.generateProcess(&ctx, 1, 0);
  ASSERT_NE(p, nullptr);
  EXPECT_TRUE(p->IsOwnedByScheduler());
  delete p;
}

} // namespace SchedulerGenerateProcess

namespace SchedulerSleepRelinquish {

// A process containing a SLEEP statement must relinquish the CPU core,
// increment its sleep ticks in the scheduler loop, and wake up to finish.
TEST(SchedulerSleepRelinquish, ProcessSleepsAndWakesUpCorrectly) {
  AlgoContext ctx = makeTestCtx();
  ctx.num_cpu = 1; // single core to make core occupancy issues obvious
  prosched::Scheduler scheduler(ctx);

  prosched::Process *p = new prosched::Process("sleepy_proc", 1, 0);
  AddRaw(*p, "PRINT(\"before\")");
  AddRaw(*p, "SLEEP(5)");
  AddRaw(*p, "PRINT(\"after\")");

  scheduler.AddProcess(p);
  scheduler.Start();
  scheduler.StopGenerating();

  // Wait long enough for the process to run, sleep for 5 ticks, wake, and
  // complete. 50 ms is generous with kTickDurationMs = 0.
  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  scheduler.Stop();

  EXPECT_TRUE(p->IsFinished());
  EXPECT_EQ(p->GetState(), prosched::FINISHED);

  auto logs = p->GetLogs();
  bool found_before = false;
  bool found_sleep = false;
  bool found_after = false;

  for (const auto &log : logs) {
    if (log.find("before") != std::string::npos)
      found_before = true;
    if (log.find("SLEEP") != std::string::npos)
      found_sleep = true;
    if (log.find("after") != std::string::npos)
      found_after = true;
  }

  EXPECT_TRUE(found_before);
  EXPECT_TRUE(found_sleep);
  EXPECT_TRUE(found_after);
}

} // namespace SchedulerSleepRelinquish

namespace SchedulerFCFS {

  // A queued process gets dispatched and runs to completion under FCFS
  TEST(SchedulerFCFS, DispatchesToIdleWorker) {
    prosched::Scheduler scheduler(makeSmallCtx("fcfs"));
    prosched::Process *p = new prosched::Process("task", 1, 0);
    AddRaw(*p, "PRINT(\"a\")");
    AddRaw(*p, "PRINT(\"b\")");
    p->SetOwnedByScheduler(true);
    scheduler.AddProcess(p);

    scheduler.Start();

    auto deadline =
        std::chrono::steady_clock::now() + std::chrono::milliseconds(100);
    while (!p->IsFinished() && std::chrono::steady_clock::now() < deadline)
      std::this_thread::yield();

    scheduler.Stop();
    EXPECT_TRUE(p->IsFinished());
  }

  // With one CPU and two queued processes both must finish (FCFS skips busy core)
  TEST(SchedulerFCFS, BothProcessesFinishWithOneCPU) {
    prosched::Scheduler scheduler(makeSmallCtx("fcfs"));
    prosched::Process *p1 = new prosched::Process("first", 1, 0);
    prosched::Process *p2 = new prosched::Process("second", 2, 0);
    AddRaw(*p1, "PRINT(\"a\")");
    AddRaw(*p2, "PRINT(\"b\")");
    p1->SetOwnedByScheduler(true);
    p2->SetOwnedByScheduler(true);
    scheduler.AddProcess(p1);
    scheduler.AddProcess(p2);

    scheduler.Start();

    auto deadline =
        std::chrono::steady_clock::now() + std::chrono::milliseconds(100);
    while ((!p1->IsFinished() || !p2->IsFinished()) &&
          std::chrono::steady_clock::now() < deadline)
      std::this_thread::yield();

    scheduler.Stop();
    EXPECT_TRUE(p1->IsFinished());
    EXPECT_TRUE(p2->IsFinished());
  }

} // namespace SchedulerFCFS

namespace SchedulerRoundRobin {

  // A process with more instructions than the quantum finishes after preemptions
  TEST(SchedulerRoundRobin, ProcessFinishesAfterPreemptions) {
    prosched::Scheduler scheduler(makeSmallCtx("rr", 1, 2));
    prosched::Process *p = new prosched::Process("rr_task", 1, 0);
    for (int i = 0; i < 6; i++)
      AddRaw(*p, "PRINT(\"tick\")");
    p->SetOwnedByScheduler(true);
    scheduler.AddProcess(p);

    scheduler.Start();

    auto deadline =
        std::chrono::steady_clock::now() + std::chrono::milliseconds(200);
    while (!p->IsFinished() && std::chrono::steady_clock::now() < deadline)
      std::this_thread::yield();

    scheduler.Stop();
    EXPECT_TRUE(p->IsFinished());
    EXPECT_EQ(p->GetCurrentInstructionIndex(), 6);
  }

// Two processes share one CPU via time slices and both finish
TEST(SchedulerRoundRobin, MultipleProcessesFinish) {
  prosched::Scheduler scheduler(makeSmallCtx("rr", 1, 2));
  prosched::Process *p1 = new prosched::Process("rr1", 1, 0);
  prosched::Process *p2 = new prosched::Process("rr2", 2, 0);
  for (int i = 0; i < 4; i++)
    AddRaw(*p1, "PRINT(\"a\")");
  for (int i = 0; i < 4; i++)
    AddRaw(*p2, "PRINT(\"b\")");
  p1->SetOwnedByScheduler(true);
  p2->SetOwnedByScheduler(true);
  scheduler.AddProcess(p1);
  scheduler.AddProcess(p2);

  scheduler.Start();

  auto deadline =
      std::chrono::steady_clock::now() + std::chrono::milliseconds(200);
  while ((!p1->IsFinished() || !p2->IsFinished()) &&
         std::chrono::steady_clock::now() < deadline)
    std::this_thread::yield();

  scheduler.Stop();
  EXPECT_TRUE(p1->IsFinished());
  EXPECT_TRUE(p2->IsFinished());
}

} // namespace SchedulerRoundRobin

namespace SchedulerCollectPreemptedCycle {

  // CollectPreemptedCycle with no workers must not crash
  TEST(SchedulerCollectPreemptedCycle, NoWorkersNoCrash) {
    prosched::Scheduler scheduler(makeTestCtx());
    EXPECT_NO_THROW(scheduler.CollectPreemptedCycle());
  }

} // namespace SchedulerCollectPreemptedCycle

namespace SchedulerUpdateSleepingProcessesCycle {

  // cyclesRemainingForSleep decrements by 1 on each call while still > 0
  TEST(SchedulerUpdateSleepingProcessesCycle, DecrementsEachCall) {
    prosched::Scheduler scheduler(makeTestCtx());
    prosched::Process p("sleeper", 1, 0);
    AddRaw(p, "SLEEP(3)");
    AddRaw(p, "PRINT(\"x\")");
    scheduler.AddProcess(&p);
    p.ExecuteInstructions(0); // execute SLEEP → WAITING, cyclesRemaining=3

    ASSERT_EQ(p.GetState(), prosched::WAITING);
    ASSERT_EQ(p.GetCyclesRemainingForSleep(), 3);

    scheduler.UpdateSleepingProcessesCycle();
    EXPECT_EQ(p.GetCyclesRemainingForSleep(), 2);

    scheduler.UpdateSleepingProcessesCycle();
    EXPECT_EQ(p.GetCyclesRemainingForSleep(), 1);
  }

  // State transitions WAITING → READY when countdown reaches 0 and instructions remain
  TEST(SchedulerUpdateSleepingProcessesCycle, BecomesReadyWhenSleepEnds) {
    prosched::Scheduler scheduler(makeTestCtx());
    prosched::Process p("sleeper", 1, 0);
    AddRaw(p, "SLEEP(2)");
    AddRaw(p, "PRINT(\"after\")");
    scheduler.AddProcess(&p);
    p.ExecuteInstructions(0); // WAITING, cyclesRemaining=2

    scheduler.UpdateSleepingProcessesCycle(); // 2 → 1, still WAITING
    EXPECT_EQ(p.GetState(), prosched::WAITING);

    scheduler.UpdateSleepingProcessesCycle(); // 1 → 0, no more sleep → READY
    EXPECT_EQ(p.GetState(), prosched::READY);
  }

  // State transitions WAITING → FINISHED when countdown reaches 0 and no instructions remain
  TEST(SchedulerUpdateSleepingProcessesCycle, BecomesFinishedWhenNoInstructions) {
    prosched::Scheduler scheduler(makeTestCtx());
    prosched::Process p("sleeper", 1, 0);
    AddRaw(p, "SLEEP(1)");
    scheduler.AddProcess(&p);
    p.ExecuteInstructions(0); // WAITING, cyclesRemaining=1, index=1==total

    ASSERT_EQ(p.GetState(), prosched::WAITING);

    scheduler.UpdateSleepingProcessesCycle(); // 1 → 0, index >= total → FINISHED
    EXPECT_EQ(p.GetState(), prosched::FINISHED);
  }

} // namespace SchedulerUpdateSleepingProcessesCycle

namespace SchedulerGenerateProcessesCycle {

  // A process is added when tick falls on the batch_process_frequency boundary
  TEST(SchedulerGenerateProcessesCycle, AddsProcessOnMatchingTick) {
    AlgoContext ctx = makeTestCtx();
    ctx.batch_process_frequency = 5;
    prosched::Scheduler scheduler(ctx);
    scheduler.ResumeGenerating();

    int before = (int)scheduler.GetAllProcesses().size();
    scheduler.GenerateProcessesCycle(5); // 5 % 5 == 0
    EXPECT_EQ((int)scheduler.GetAllProcesses().size(), before + 1);
  }

  // No process is added when tick does not fall on the frequency boundary
  TEST(SchedulerGenerateProcessesCycle, SkipsNonMatchingTick) {
    AlgoContext ctx = makeTestCtx();
    ctx.batch_process_frequency = 5;
    prosched::Scheduler scheduler(ctx);
    scheduler.ResumeGenerating();

    int before = (int)scheduler.GetAllProcesses().size();
    scheduler.GenerateProcessesCycle(3); // 3 % 5 != 0
    EXPECT_EQ((int)scheduler.GetAllProcesses().size(), before);
  }

  // generatingProcesses defaults to false — no process created without ResumeGenerating
  TEST(SchedulerGenerateProcessesCycle, DefaultStatePreventsGeneration) {
    AlgoContext ctx = makeTestCtx();
    ctx.batch_process_frequency = 1;
    prosched::Scheduler scheduler(ctx);
    // generatingProcesses == false by default

    int before = (int)scheduler.GetAllProcesses().size();
    scheduler.GenerateProcessesCycle(1);
    EXPECT_EQ((int)scheduler.GetAllProcesses().size(), before);
  }

} // namespace SchedulerGenerateProcessesCycle

namespace SchedulerGeneratingToggle {

  // StopGenerating prevents new processes even when tick would match
  TEST(SchedulerGeneratingToggle, StopGeneratingPreventsNewProcesses) {
    AlgoContext ctx = makeTestCtx();
    ctx.batch_process_frequency = 1;
    prosched::Scheduler scheduler(ctx);
    scheduler.ResumeGenerating();
    scheduler.StopGenerating();

    int before = (int)scheduler.GetAllProcesses().size();
    scheduler.GenerateProcessesCycle(1);
    EXPECT_EQ((int)scheduler.GetAllProcesses().size(), before);
  }

  // ResumeGenerating re-enables generation after StopGenerating
  TEST(SchedulerGeneratingToggle, ResumeGeneratingAllowsNewProcesses) {
    AlgoContext ctx = makeTestCtx();
    ctx.batch_process_frequency = 1;
    prosched::Scheduler scheduler(ctx);
    scheduler.StopGenerating();
    scheduler.ResumeGenerating();

    int before = (int)scheduler.GetAllProcesses().size();
    scheduler.GenerateProcessesCycle(1);
    EXPECT_EQ((int)scheduler.GetAllProcesses().size(), before + 1);
  }

} // namespace SchedulerGeneratingToggle

namespace SchedulerFindProcessByName {

  TEST(SchedulerFindProcessByName, FindsExistingProcess) {
    prosched::Scheduler scheduler(makeTestCtx());
    prosched::Process p("target_proc", 1, 0);
    scheduler.AddProcess(&p);

    prosched::Process *found = scheduler.FindProcessByName("target_proc");
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->GetName(), "target_proc");
  }

  TEST(SchedulerFindProcessByName, ReturnsNullForUnknownName) {
    prosched::Scheduler scheduler(makeTestCtx());

    prosched::Process *found = scheduler.FindProcessByName("nonexistent");
    EXPECT_EQ(found, nullptr);
  }

  // FindProcessByName returns nullptr for a process that has already finished
  TEST(SchedulerFindProcessByName, ReturnsNullForFinishedProcess) {
    prosched::Scheduler scheduler(makeTestCtx());
    prosched::Process p("done_proc", 1, 0);
    AddRaw(p, "PRINT(\"x\")");
    scheduler.AddProcess(&p);
    p.ExecuteInstructions(0);
    ASSERT_TRUE(p.IsFinished());

    prosched::Process *found = scheduler.FindProcessByName("done_proc");
    EXPECT_EQ(found, nullptr);
  }

} // namespace SchedulerFindProcessByName

namespace SchedulerCreateNamedProcess {

  TEST(SchedulerCreateNamedProcess, HasCorrectName) {
    prosched::Scheduler scheduler(makeTestCtx());
    prosched::Process *p = scheduler.CreateNamedProcess("my_process");
    ASSERT_NE(p, nullptr);
    EXPECT_EQ(p->GetName(), "my_process");
    delete p;
}

  TEST(SchedulerCreateNamedProcess, InstructionCountInRange) {
    AlgoContext ctx = makeTestCtx();
    prosched::Scheduler scheduler(ctx);
    prosched::Process *p = scheduler.CreateNamedProcess("count_test");
    ASSERT_NE(p, nullptr);
    EXPECT_GE(p->GetTotalInstructions(), ctx.min_ins);
    EXPECT_LE(p->GetTotalInstructions(), ctx.max_ins);
    delete p;
  }

  TEST(SchedulerCreateNamedProcess, IsOwnedByScheduler) {
    prosched::Scheduler scheduler(makeTestCtx());
    prosched::Process *p = scheduler.CreateNamedProcess("owned");
    ASSERT_NE(p, nullptr);
    EXPECT_TRUE(p->IsOwnedByScheduler());
    delete p;
  }

  // Consecutive calls must produce strictly increasing PIDs
  TEST(SchedulerCreateNamedProcess, PIDIncrements) {
    prosched::Scheduler scheduler(makeTestCtx());
    prosched::Process *p1 = scheduler.CreateNamedProcess("first");
    prosched::Process *p2 = scheduler.CreateNamedProcess("second");
    ASSERT_NE(p1, nullptr);
    ASSERT_NE(p2, nullptr);
    EXPECT_EQ(p2->GetPID(), p1->GetPID() + 1);
    delete p1;
    delete p2;
  }

} // namespace SchedulerCreateNamedProcess

namespace SchedulerTriggerTick {

  // TriggerWorkersTick with no workers (empty vector) returns immediately
  TEST(SchedulerTriggerTick, NoWorkersNoCrash) {
    prosched::Scheduler scheduler(makeTestCtx());
    // Scheduler not started → workers vector empty, running=false → exits immediately
    EXPECT_NO_THROW(scheduler.TriggerWorkersTick(1));
  }

  // NotifyWorkerDone can be called on an idle scheduler without crashing
  TEST(SchedulerTriggerTick, NotifyWorkerDoneNoCrash) {
    prosched::Scheduler scheduler(makeTestCtx());
    EXPECT_NO_THROW(scheduler.NotifyWorkerDone());
  }

} // namespace SchedulerTriggerTick

namespace SchedulerPrintProcessesContent {

  TEST(SchedulerPrintProcessesContent, ContainsRunningSection) {
    prosched::Scheduler scheduler(makeTestCtx());
    std::ostringstream out;
    scheduler.PrintProcesses(out);
    EXPECT_NE(out.str().find("Running processes:"), std::string::npos);
  }

  TEST(SchedulerPrintProcessesContent, ContainsFinishedSection) {
    prosched::Scheduler scheduler(makeTestCtx());
    std::ostringstream out;
    scheduler.PrintProcesses(out);
    EXPECT_NE(out.str().find("Finished processes:"), std::string::npos);
  }

  TEST(SchedulerPrintProcessesContent, ContainsCPUUtilizationFields) {
    prosched::Scheduler scheduler(makeTestCtx());
    std::ostringstream out;
    scheduler.PrintProcesses(out);
    EXPECT_NE(out.str().find("CPU utilization:"), std::string::npos);
    EXPECT_NE(out.str().find("Cores used:"), std::string::npos);
    EXPECT_NE(out.str().find("Cores available:"), std::string::npos);
  }

  // No workers started → utilization must be 0%
  TEST(SchedulerPrintProcessesContent, ZeroUtilizationWhenIdle) {
    prosched::Scheduler scheduler(makeTestCtx());
    std::ostringstream out;
    scheduler.PrintProcesses(out);
    EXPECT_NE(out.str().find("0%"), std::string::npos);
  }

  TEST(SchedulerPrintProcessesContent, ShowsProcessNameInOutput) {
    prosched::Scheduler scheduler(makeTestCtx());
    prosched::Process p("named_proc", 1, 0);
    AddRaw(p, "PRINT(\"x\")");
    scheduler.AddProcess(&p);

    std::ostringstream out;
    scheduler.PrintProcesses(out);
    EXPECT_NE(out.str().find("named_proc"), std::string::npos);
  }

  // A finished process must appear after the "Finished processes:" label
  TEST(SchedulerPrintProcessesContent, FinishedProcessAppearsAfterFinishedLabel) {
    prosched::Scheduler scheduler(makeTestCtx());
    prosched::Process p("done", 1, 0);
    AddRaw(p, "PRINT(\"x\")");
    scheduler.AddProcess(&p);
    p.ExecuteInstructions(0);
    ASSERT_TRUE(p.IsFinished());

    std::ostringstream out;
    scheduler.PrintProcesses(out);
    std::string result = out.str();
    size_t fin_pos = result.find("Finished processes:");
    size_t name_pos = result.find("done");
    EXPECT_NE(fin_pos, std::string::npos);
    EXPECT_NE(name_pos, std::string::npos);
    EXPECT_GT(name_pos, fin_pos);
  }

} // namespace SchedulerPrintProcessesContent
