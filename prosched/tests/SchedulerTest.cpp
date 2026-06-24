// #include "scheduler/Scheduler.h"
// #include "config.h"
// #include "context.h"
// #include "scheduler/process/Process.h"
// #include <chrono>
// #include <gtest/gtest.h>
// #include <thread>

// static AlgoContext makeTestCtx() {
//   ConfigStruct *cs = makeDefault();
//   cs->scheduler = "fcfs";
//   AlgoContext ctx = AlgoContext::buildConfig(cs);
//   delete cs;
//   return ctx;
// }

// namespace SchedulerAddProcess {

//     // AddProcess returns the same pointer that was passed in
//   TEST(SchedulerAddProcess, ReturnsSameProcess) {
//     prosched::Scheduler scheduler(makeTestCtx());
//     prosched::Process p("test_process", 1, 0);

//     prosched::Process *result = scheduler.AddProcess(&p);

//     ASSERT_NE(result, nullptr);
//     EXPECT_EQ(result->GetPID(), p.GetPID());
//     EXPECT_EQ(result->GetName(), p.GetName());
//   }

//   // Adding multiple processes should each return their own identity
//   TEST(SchedulerAddProcess, MultipleProcessesReturnCorrectly) {
//     prosched::Scheduler scheduler(makeTestCtx());
//     prosched::Process p1("proc_alpha", 1, 0);
//     prosched::Process p2("proc_beta", 2, 1);

//     prosched::Process *r1 = scheduler.AddProcess(&p1);
//     prosched::Process *r2 = scheduler.AddProcess(&p2);

//     ASSERT_NE(r1, nullptr);
//     ASSERT_NE(r2, nullptr);
//     EXPECT_EQ(r1->GetPID(), 1);
//     EXPECT_EQ(r1->GetName(), "proc_alpha");
//     EXPECT_EQ(r2->GetPID(), 2);
//     EXPECT_EQ(r2->GetName(), "proc_beta");
//   }

//   // Empty process name is still accepted
//   TEST(SchedulerAddProcess, EmptyProcessName) {
//     prosched::Scheduler scheduler(makeTestCtx());
//     prosched::Process p("", 1, 0);

//     prosched::Process *result = scheduler.AddProcess(&p);

//     ASSERT_NE(result, nullptr);
//     EXPECT_EQ(result->GetName(), "");
//   }

//   // PID 0 is a valid boundary value
//   TEST(SchedulerAddProcess, PIDZero) {
//     prosched::Scheduler scheduler(makeTestCtx());
//     prosched::Process p("pid_zero", 0, 0);

//     prosched::Process *result = scheduler.AddProcess(&p);

//     ASSERT_NE(result, nullptr);
//     EXPECT_EQ(result->GetPID(), 0);
//   }

//   // Same pointer added twice — scheduler must not crash or corrupt
//   TEST(SchedulerAddProcess, DuplicatePID) {
//     prosched::Scheduler scheduler(makeTestCtx());
//     prosched::Process p("dup", 5, 0);

//     prosched::Process *r1 = scheduler.AddProcess(&p);
//     prosched::Process *r2 = scheduler.AddProcess(&p);

//     ASSERT_NE(r1, nullptr);
//     ASSERT_NE(r2, nullptr);
//     EXPECT_EQ(r1->GetPID(), r2->GetPID());
//     EXPECT_EQ(r1->GetName(), r2->GetName());
//   }

//   // INT_MAX arrival tick must not overflow or crash
//   TEST(SchedulerAddProcess, LargeArrivalTick) {
//     prosched::Scheduler scheduler(makeTestCtx());
//     prosched::Process p("late_proc", 1, INT_MAX);

//     prosched::Process *result = scheduler.AddProcess(&p);

//     ASSERT_NE(result, nullptr);
//     EXPECT_EQ(result->GetPID(), 1);
//     EXPECT_EQ(result->GetName(), "late_proc");
//   }

//   // Adding more processes than num_cpu must not drop any
//   TEST(SchedulerAddProcess, MoreProcessesThanCores) {
//     prosched::Scheduler scheduler(makeTestCtx());
//     const int count = 10;
//     std::vector<prosched::Process> procs;
//     procs.reserve(count);
//     for (int i = 0; i < count; i++)
//       procs.emplace_back("p" + std::to_string(i), i, i);

//     for (int i = 0; i < count; i++) {
//       prosched::Process *result = scheduler.AddProcess(&procs[i]);
//       ASSERT_NE(result, nullptr) << "Failed at index " << i;
//       EXPECT_EQ(result->GetPID(), i);
//       EXPECT_EQ(result->GetName(), "p" + std::to_string(i));
//     }
//   }

//   // Out-of-chronological-order arrival ticks must both be accepted
//   TEST(SchedulerAddProcess, OutOfOrderArrivalTick) {
//     prosched::Scheduler scheduler(makeTestCtx());
//     prosched::Process late("late_proc", 1, 100);
//     prosched::Process early("early_proc", 2, 0);

//     prosched::Process *r_late = scheduler.AddProcess(&late);
//     prosched::Process *r_early = scheduler.AddProcess(&early);

//     ASSERT_NE(r_late, nullptr);
//     ASSERT_NE(r_early, nullptr);
//     EXPECT_EQ(r_late->GetName(), "late_proc");
//     EXPECT_EQ(r_early->GetName(), "early_proc");
//   }

//   // nullptr input must return nullptr and not crash
//   TEST(SchedulerAddProcess, NullProcessReturnsNull) {
//     prosched::Scheduler scheduler(makeTestCtx());

//     prosched::Process *result = scheduler.AddProcess(nullptr);

//     EXPECT_EQ(result, nullptr);
//   }

// // AddProcess acquires schedulerMutex; SchedulerLoop also acquires it for
// // process generation and FCFS — must return the same pointer even under contention
// TEST(SchedulerAddProcess, AddWhileRunningReturnsSamePointer) {
//   prosched::Scheduler scheduler(makeTestCtx());
//   scheduler.Start();

//   prosched::Process p("live_add", 1, 0);
//   p.AddInstruction("PRINT(\"hi\")");

//   prosched::Process *result = scheduler.AddProcess(&p);
//   EXPECT_EQ(result, &p);

//   scheduler.Stop();
// }

// } // namespace SchedulerAddProcess

// namespace SchedulerStartStop {

//   // Scheduler thread is not started on construction
//   TEST(SchedulerStartStop, IsRunningFalseOnConstruction) {
//     prosched::Scheduler scheduler(makeTestCtx());
//     EXPECT_FALSE(scheduler.IsRunning());
//   }

//   // Start() returns true on a fresh scheduler
//   TEST(SchedulerStartStop, StartReturnsTrueOnSuccess) {
//     prosched::Scheduler scheduler(makeTestCtx());

//     bool result = scheduler.Start();
//     scheduler.Stop();

//     EXPECT_TRUE(result);
//   }

//   // IsRunning reflects the scheduler being active after Start
//   TEST(SchedulerStartStop, IsRunningTrueAfterStart) {
//     prosched::Scheduler scheduler(makeTestCtx());

//     scheduler.Start();
//     EXPECT_TRUE(scheduler.IsRunning());

//     scheduler.Stop();
//   }

//   // Start() returns false and leaves state unchanged when already running
//   TEST(SchedulerStartStop, StartReturnsFalseIfAlreadyRunning) {
//     prosched::Scheduler scheduler(makeTestCtx());

//     scheduler.Start();
//     bool result = scheduler.Start(); // second call

//     EXPECT_FALSE(result);
//     EXPECT_TRUE(scheduler.IsRunning()); // still running

//     scheduler.Stop();
//   }

//   // IsRunning drops to false once Stop completes
//   TEST(SchedulerStartStop, IsRunningFalseAfterStop) {
//     prosched::Scheduler scheduler(makeTestCtx());

//     scheduler.Start();
//     scheduler.Stop();

//     EXPECT_FALSE(scheduler.IsRunning());
//   }

//   // Stop() on a scheduler that never started must not crash
//   TEST(SchedulerStartStop, StopWithoutStartDoesNotCrash) {
//     prosched::Scheduler scheduler(makeTestCtx());
//     EXPECT_NO_THROW(scheduler.Stop());
//     EXPECT_FALSE(scheduler.IsRunning());
//   }

//   // Stop() while the SchedulerLoop is actively ticking must not deadlock
//   TEST(SchedulerStartStop, StopWhileSchedulingDoesNotDeadlock) {
//     prosched::Scheduler scheduler(makeTestCtx());

//     scheduler.Start();
//     std::this_thread::sleep_for(std::chrono::milliseconds(100));
//     scheduler.Stop();

//     EXPECT_FALSE(scheduler.IsRunning());
//   }

//   // Stop() while workers are mid-execution must still clean up cleanly
//   TEST(SchedulerStartStop, StopWhileWorkersExecutingDoesNotDeadlock) {
//     prosched::Scheduler scheduler(makeTestCtx());

//     // Pre-load processes before starting so workers get busy immediately
//     for (int i = 1; i <= 4; i++) {
//       prosched::Process *p =
//           new prosched::Process("pre_" + std::to_string(i), i, 0);
//       for (int j = 0; j < 20; j++)
//         p->AddInstruction("PRINT(\"tick\")");
//       scheduler.AddProcess(p);
//     }

//     scheduler.Start();
//     std::this_thread::sleep_for(std::chrono::milliseconds(50));
//     scheduler.Stop();

//     EXPECT_FALSE(scheduler.IsRunning());
//   }

//   // Start/Stop can cycle multiple times without deadlock or corrupted state
//   TEST(SchedulerStartStop, MultipleRestartCycles) {
//     prosched::Scheduler scheduler(makeTestCtx());

//     for (int i = 0; i < 3; i++) {
//       bool started = scheduler.Start();
//       EXPECT_TRUE(started) << "Start failed on cycle " << i;
//       EXPECT_TRUE(scheduler.IsRunning());

//       scheduler.Stop();
//       EXPECT_FALSE(scheduler.IsRunning());
//     }
//   }

// } // namespace SchedulerStartStop

// namespace SchedulerPrintProcesses {

//   // Printing with no processes registered must not crash
//   TEST(SchedulerPrintProcesses, EmptyListDoesNotCrash) {
//     prosched::Scheduler scheduler(makeTestCtx());
//     EXPECT_NO_THROW(scheduler.PrintProcesses());
//   }

//   // Printing after AddProcess must not crash
//   TEST(SchedulerPrintProcesses, PrintAfterAddDoesNotCrash) {
//     prosched::Scheduler scheduler(makeTestCtx());
//     prosched::Process p("print_test", 1, 0);
//     scheduler.AddProcess(&p);

//     EXPECT_NO_THROW(scheduler.PrintProcesses());
//   }

//   // PrintProcesses while the scheduler is running must not deadlock
//   TEST(SchedulerPrintProcesses, PrintWhileRunningDoesNotDeadlock) {
//     prosched::Scheduler scheduler(makeTestCtx());

//     scheduler.Start();
//     std::this_thread::sleep_for(std::chrono::milliseconds(50));

//     EXPECT_NO_THROW(scheduler.PrintProcesses());

//     scheduler.Stop();
//   }

// // PrintProcesses holds schedulerMutex then acquires workerMutex via
// // GetCurrentProcess — two concurrent callers must not produce lock-order inversion;
// // scheduler must still be running correctly after both complete
// TEST(SchedulerPrintProcesses, ConcurrentPrintLeavesSchedulerRunning) {
//   prosched::Scheduler scheduler(makeTestCtx());
//   scheduler.Start();
//   std::this_thread::sleep_for(std::chrono::milliseconds(50));

//   std::thread t1([&] { scheduler.PrintProcesses(); });
//   std::thread t2([&] { scheduler.PrintProcesses(); });

//   t1.join();
//   t2.join();

//   EXPECT_TRUE(scheduler.IsRunning());

//   scheduler.Stop();
// }

// } // namespace SchedulerPrintProcesses

// namespace SchedulerGenerateProcess {

//   // generateProcess() is public and does NOT increment nextPID — that happens
//   // only inside SchedulerLoop. A fresh Scheduler has nextPID == 1, so the first
//   // call always produces name "process1" regardless of the pid argument.

//   // generateProcess always returns a valid heap-allocated Process
//   TEST(SchedulerGenerateProcess, ReturnsNonNull) {
//     prosched::Scheduler scheduler(makeTestCtx());
//     AlgoContext ctx = makeTestCtx();
//     prosched::Process *p = scheduler.generateProcess(&ctx, 1, 0);
//     ASSERT_NE(p, nullptr);
//     delete p;
//   }

//   // Name uses nextPID (starts at 1), not the pid parameter
//   TEST(SchedulerGenerateProcess, NameFollowsProcessNPattern) {
//     prosched::Scheduler scheduler(makeTestCtx());
//     AlgoContext ctx = makeTestCtx();
//     prosched::Process *p = scheduler.generateProcess(&ctx, 1, 0);
//     ASSERT_NE(p, nullptr);
//     EXPECT_EQ(p->GetName(), "process1");
//     delete p;
//   }

//   // Exactly NUM_PRINT_INSTRUCTIONS (100) PRINT statements are added
//   TEST(SchedulerGenerateProcess, HasExactlyNumPrintInstructions) {
//     prosched::Scheduler scheduler(makeTestCtx());
//     AlgoContext ctx = makeTestCtx();
//     prosched::Process *p = scheduler.generateProcess(&ctx, 1, 0);
//     ASSERT_NE(p, nullptr);
//     EXPECT_EQ(p->GetTotalInstructions(), prosched::kNumPrintInstructions);
//     delete p;
//   }

//   // Processes are flagged for cleanup by the Scheduler destructor
//   TEST(SchedulerGenerateProcess, IsOwnedByScheduler) {
//     prosched::Scheduler scheduler(makeTestCtx());
//     AlgoContext ctx = makeTestCtx();
//     prosched::Process *p = scheduler.generateProcess(&ctx, 1, 0);
//     ASSERT_NE(p, nullptr);
//     EXPECT_TRUE(p->IsOwnedByScheduler());
//     delete p;
//   }

// } // namespace SchedulerGenerateProcess

// namespace SchedulerSleepRelinquish {

//   // A process containing a SLEEP statement must relinquish the CPU core,
//   // increment its sleep ticks in the scheduler loop, and wake up to finish.
//   TEST(SchedulerSleepRelinquish, ProcessSleepsAndWakesUpCorrectly) {
//     AlgoContext ctx = makeTestCtx();
//     ctx.numCpu = 1; // single core to make core occupancy issues obvious
//     prosched::Scheduler scheduler(ctx);

//     prosched::Process *p = new prosched::Process("sleepy_proc", 1, 0);
//     p->AddInstruction("PRINT(\"before\")");
//     p->AddInstruction("SLEEP(5)");
//     p->AddInstruction("PRINT(\"after\")");

//     scheduler.AddProcess(p);
//     scheduler.Start();
//     scheduler.StopGenerating();


//     // Sleep long enough for the process to run, sleep for 5 ticks, wake up, and complete
//     std::this_thread::sleep_for(std::chrono::milliseconds(200));

//     scheduler.Stop();

//     EXPECT_TRUE(p->IsFinished());
//     EXPECT_EQ(p->GetState(), prosched::Process::FINISHED);

//     auto logs = p->GetLogs();
//     bool found_before = false;
//     bool found_sleep = false;
//     bool found_after = false;

//     for (const auto &log : logs) {
//         if (log.find("before") != std::string::npos) found_before = true;
//         if (log.find("SLEEP") != std::string::npos) found_sleep = true;
//         if (log.find("after") != std::string::npos) found_after = true;
//     }

//     EXPECT_TRUE(found_before);
//     EXPECT_TRUE(found_sleep);
//     EXPECT_TRUE(found_after);
//   }

// } // namespace SchedulerSleepRelinquish

