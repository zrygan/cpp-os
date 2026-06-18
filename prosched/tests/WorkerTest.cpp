#include <gtest/gtest.h>
#include <chrono>
#include <thread>
#include "scheduler/worker/Worker.h"
#include "scheduler/process/Process.h"

static AlgoContext makeTestCtx() {
    ConfigStruct *cs = makeDefault();
    cs->scheduler = "fcfs";
    AlgoContext ctx = AlgoContext::buildConfig(cs);
    delete cs;
    return ctx;
}

namespace WorkerAssignProcess {

  // Idle worker — returns the process that was passed in
  TEST(WorkerAssignProcess, ReturnsProcessWhenIdle) {
      prosched::Worker w(1, makeTestCtx());
      prosched::Process p("assign_1", 1, 0);

      prosched::Process *result = w.AssignProcess(&p);

      EXPECT_EQ(result, &p);
  }

  // Nullptr input on an idle worker — returns nullptr, worker stays idle
  TEST(WorkerAssignProcess, NullInputOnIdleWorkerReturnsNull) {
      prosched::Worker w(1, makeTestCtx());

      prosched::Process *result = w.AssignProcess(nullptr);

      EXPECT_EQ(result, nullptr);
      EXPECT_FALSE(w.IsBusy());
  }

  // Worker already has a process — second assign must return nullptr (busy guard)
  TEST(WorkerAssignProcess, ReturnsNullWhenBusy) {
      prosched::Worker w(1, makeTestCtx());
      prosched::Process p1("assign_busy_1", 1, 0);
      prosched::Process p2("assign_busy_2", 2, 0);

      w.AssignProcess(&p1);
      prosched::Process *result = w.AssignProcess(&p2);

      EXPECT_EQ(result, nullptr);
  }

  // Running but idle worker (no process yet) — assign should succeed
  TEST(WorkerAssignProcess, AssignToRunningIdleWorkerSucceeds) {
      prosched::Worker w(1, makeTestCtx());
      prosched::Process p("assign_running", 1, 0);

      bool start = w.Start();
      prosched::Process *result = w.AssignProcess(&p);
      bool stop = w.Stop();

      EXPECT 
      EXPECT_EQ(result, &p);
  }

  // After process finishes and thread clears it, a new process can be assigned
  TEST(WorkerAssignProcess, CanAssignAgainAfterProcessFinishes) {
      prosched::Worker w(1, makeTestCtx());
      prosched::Process p1("assign_after_1", 1, 0);
      prosched::Process p2("assign_after_2", 2, 0);
      p1.AddInstruction("PRINT(\"done\")");

      w.AssignProcess(&p1);
      w.Start();
      std::this_thread::sleep_for(std::chrono::milliseconds(200));

      // p1 should be finished and currentProcess should be cleared
      ASSERT_TRUE(p1.IsFinished());

      prosched::Process *result = w.AssignProcess(&p2);
      w.Stop();

      EXPECT_EQ(result, &p2);
  }

} // namespace WorkerAssignProcess


namespace WorkerIsBusy {

  TEST(WorkerIsBusy, FalseOnConstruction) {
      prosched::Worker w(1, makeTestCtx());
      EXPECT_FALSE(w.IsBusy());
  }

  TEST(WorkerIsBusy, TrueAfterValidAssign) {
      prosched::Worker w(1, makeTestCtx());
      prosched::Process p("busy_1", 1, 0);

      w.AssignProcess(&p);
      EXPECT_TRUE(w.IsBusy());
  }

  // Assigning nullptr on an idle worker changes nothing — stays not busy
  TEST(WorkerIsBusy, FalseAfterNullAssignOnIdleWorker) {
      prosched::Worker w(1, makeTestCtx());

      w.AssignProcess(nullptr);
      EXPECT_FALSE(w.IsBusy());
  }

  // Once the process finishes, the thread clears currentProcess — worker goes idle
  TEST(WorkerIsBusy, FalseAfterProcessFinishes) {
      prosched::Worker w(1, makeTestCtx());
      prosched::Process p("busy_thread", 1, 0);
      p.AddInstruction("PRINT(\"hi\")");

      w.AssignProcess(&p);
      w.Start();
      std::this_thread::sleep_for(std::chrono::milliseconds(200));

      EXPECT_FALSE(w.IsBusy());

      w.Stop();
  }

} // namespace WorkerIsBusy


namespace WorkerIsRunning {

  TEST(WorkerIsRunning, FalseOnConstruction) {
      prosched::Worker w(1, makeTestCtx());
      EXPECT_FALSE(w.IsRunning());
  }

  TEST(WorkerIsRunning, TrueAfterStart) {
      prosched::Worker w(1, makeTestCtx());

      w.Start();
      EXPECT_TRUE(w.IsRunning());

      w.Stop();
  }

  TEST(WorkerIsRunning, FalseAfterStop) {
      prosched::Worker w(1, makeTestCtx());

      w.Start();
      w.Stop();

      EXPECT_FALSE(w.IsRunning());
  }

  // Starting an already-running worker returns false and stays running
  TEST(WorkerIsRunning, StartReturnsFalseIfAlreadyRunning) {
      prosched::Worker w(1, makeTestCtx());

      w.Start();
      bool result = w.Start(); // second start

      EXPECT_FALSE(result);
      EXPECT_TRUE(w.IsRunning()); // still running

      w.Stop();
  }

  // Stopping a worker that was never started returns false
  TEST(WorkerIsRunning, StopReturnsFalseIfNotRunning) {
      prosched::Worker w(1, makeTestCtx());

      bool result = w.Stop();

      EXPECT_FALSE(result);
      EXPECT_FALSE(w.IsRunning());
  }

  // Start/Stop cycle can repeat without deadlock
  TEST(WorkerIsRunning, CanRestartAfterStop) {
      prosched::Worker w(1, makeTestCtx());

      w.Start();
      w.Stop();
      bool result = w.Start();

      EXPECT_TRUE(result);
      EXPECT_TRUE(w.IsRunning());

      w.Stop();
  }

  // Start() returns true on a fresh worker
  TEST(WorkerIsRunning, StartReturnsTrueOnSuccess) {
      prosched::Worker w(1, makeTestCtx());

      bool result = w.Start();

      EXPECT_TRUE(result);

      w.Stop();
  }

  // Stop() returns true when the worker was running
  TEST(WorkerIsRunning, StopReturnsTrueOnSuccess) {
      prosched::Worker w(1, makeTestCtx());

      w.Start();
      bool result = w.Stop();

      EXPECT_TRUE(result);
  }

  // Multiple start/stop cycles must not deadlock or corrupt state
  TEST(WorkerIsRunning, MultipleRestartCycles) {
      prosched::Worker w(1, makeTestCtx());

      for (int i = 0; i < 3; i++) {
          EXPECT_TRUE(w.Start())  << "Start failed on cycle " << i;
          EXPECT_TRUE(w.IsRunning());
          EXPECT_TRUE(w.Stop())   << "Stop failed on cycle " << i;
          EXPECT_FALSE(w.IsRunning());
      }
  }

  // Stop() while the worker is actively executing a process must not deadlock
  TEST(WorkerIsRunning, StopWhileProcessingDoesNotDeadlock) {
      prosched::Worker w(1, makeTestCtx());
      prosched::Process p("stop_mid", 1, 0);

      // 50 instructions — enough that the worker won't finish before we call Stop()
      for (int i = 0; i < 50; i++)
          p.AddInstruction("PRINT(\"tick\")");

      w.AssignProcess(&p);
      w.Start();
      w.Stop(); // should return cleanly even mid-execution

      EXPECT_FALSE(w.IsRunning());
  }

} // namespace WorkerIsRunning


namespace WorkerGetCoreNum {

  TEST(WorkerGetCoreNum, ReturnsConstructedCoreNum) {
      prosched::Worker w(2, makeTestCtx());
      EXPECT_EQ(w.GetCoreNum(), 2);
  }

  TEST(WorkerGetCoreNum, DifferentWorkersHaveDifferentCoreNums) {
      prosched::Worker w1(1, makeTestCtx());
      prosched::Worker w2(3, makeTestCtx());

      EXPECT_EQ(w1.GetCoreNum(), 1);
      EXPECT_EQ(w2.GetCoreNum(), 3);
  }

} // namespace WorkerGetCoreNum


namespace WorkerGetCurrentProcess {

  TEST(WorkerGetCurrentProcess, NullWhenIdle) {
      prosched::Worker w(1, makeTestCtx());
      EXPECT_EQ(w.GetCurrentProcess(), nullptr);
  }

  TEST(WorkerGetCurrentProcess, ReturnsAssignedProcess) {
      prosched::Worker w(1, makeTestCtx());
      prosched::Process p("cur_proc", 1, 0);

      w.AssignProcess(&p);

      EXPECT_EQ(w.GetCurrentProcess(), &p);
  }

  // After process finishes, thread clears it — GetCurrentProcess returns nullptr
  TEST(WorkerGetCurrentProcess, NullAfterProcessFinishes) {
      prosched::Worker w(1, makeTestCtx());
      prosched::Process p("cur_done", 1, 0);
      p.AddInstruction("PRINT(\"bye\")");

      w.AssignProcess(&p);
      w.Start();
      std::this_thread::sleep_for(std::chrono::milliseconds(200));

      EXPECT_EQ(w.GetCurrentProcess(), nullptr);

      w.Stop();
  }

} // namespace WorkerGetCurrentProcess
