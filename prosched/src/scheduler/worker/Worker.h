#pragma once

#include <mutex>
#include <stdio.h>
#include <thread>

#include "config.h"
#include "context.h"
#include "scheduler/process/Process.h"
#include "Constants.hpp"

namespace prosched {

class Worker {
public:
  /**
   * @brief Creates a worker associated with a CPU core
   *
   * @param coreNum the identifier of the CPU core assigned to this worker
   * @param ctx Scheduling configuration
   */
  Worker(int coreNum, AlgoContext ctx)
      : coreNum(coreNum), ctx(ctx), currentProcess(nullptr), running(false) {}

  /**
   * @brief Starts worker execution
   *
   * Launches the worker thread and begins processing the assigned process
   * @return true if start is successful
   */
  bool Start() {

    std::lock_guard<std::mutex> lock(workerMutex);

    if (running) {
      std::cout << "Worker on core " << coreNum << " is already running\n";
      return false;
    }
    running = true;

    workerThread = std::thread(&Worker::ThreadTask, this);
    return true;
  }

  /**
   * @brief Stops worker execution
   *
   * Signals the worker thread to terminate
   * @return true if stop is successful
   */
  bool Stop() {
    {
      std::lock_guard<std::mutex> lock(workerMutex);
      if (!running) {
        std::cout << "Worker on core " << coreNum << " is not running\n";
        return false;
      }
      running = false;
    }

    if (workerThread.joinable()) {
      workerThread.join();
    }

    return true;
  }

  /**
   * @brief Assigns a process to the worker for execution
   *
   * The assigned process becomes the worker's current task and may
   * be executed when the worker is running.
   *
   * @param p a pointer to the Process to assign
   * @return the assigned process if successful, or nullptr if busy
   */
  prosched::Process *AssignProcess(prosched::Process *p) {
    std::lock_guard<std::mutex> lock(workerMutex);
    if (currentProcess != nullptr) {
      // std::cout << "Worker on core " << coreNum << " is already assigned a
      // process\n";
      return nullptr;
    }
    currentProcess = p;
    if (p) {
      p->AssignCore(coreNum);
      p->ResetQuantumUsed();
      p->SetCurrentInstructionCyclesLeft(0);
    }
    return p;
  }

  /**
   * @brief Preempts the current process from the worker (lock-free version)
   *
   * Must be called while holding workerMutex.
   *
   * @return the preempted process, or nullptr if none
   */
  prosched::Process *PreemptProcessUnlocked() {
    prosched::Process *p = currentProcess;
    if (p != nullptr) {
      p->SetState(prosched::Process::READY);
      p->ResetQuantumUsed();
      currentProcess = nullptr;
      preemptedProcess = p;
    }
    return p;
  }

  /**
   * @brief Preempts the current process from the worker
   *
   * Thread-safe.
   *
   * @return the preempted process, or nullptr if none
   */
  prosched::Process *PreemptProcess() {
    std::lock_guard<std::mutex> lock(workerMutex);
    return PreemptProcessUnlocked();
  }

  /**
   * @brief Checks if the worker is currently running
   *
   * @return true if the worker thread is active
   */
  bool IsRunning() const { return running; }

  /**
   * @brief Checks if the worker is currently executing a process
   *
   * @return true if a process is assigned and executing
   */
  bool IsBusy() const { 
    std::lock_guard<std::mutex> lock(workerMutex);
    return currentProcess != nullptr; 
  }

  /**
   * @brief Gets the core number associated with this worker
   *
   * @return The CPU core identifier
   */
  int GetCoreNum() const { return coreNum; }

  /**
   * @brief Gets the process currently assigned to this worker
   *
   * @return A pointer to the current Process, or nullptr if idle
   */
  prosched::Process *GetCurrentProcess() const {
    std::lock_guard<std::mutex> lock(workerMutex);
    return currentProcess;
  }
  /**
   * @brief Returns and clears the last preempted process (if any)
   * 
   * Must be called from the scheduler to re-queue preempted processes.
   * Thread-safe.
   */
  prosched::Process* GetAndClearPreemptedProcess() {
      std::lock_guard<std::mutex> lock(workerMutex);
      prosched::Process* p = preemptedProcess;
      preemptedProcess = nullptr;
      return p;
  }

  /**
   * @brief Handles time-slice preemption check for Round Robin scheduling.
   *
   * Increments quantum used and detaches the process if the quantum cycle
   * limit is reached.
   *
   * @param p Pointer to the process running on the core.
   * @return true if the process was preempted and detached; false otherwise.
   */
  bool TickPreemption(Process* p) {
    if (ctx.schedulerType == SchedulerType::RR && p->GetState() == Process::RUNNING) {
        p->IncrementQuantumUsed();
        if (p->GetQuantumUsed() >= ctx.quantumCycles) {
            PreemptProcessUnlocked();   // Sets state READY, detaches, and stores in preemptedProcess
            return true;
        }
    }
    return false;
  }

  /**
   * @brief Manages execution delay cycles and triggers instruction execution.
   *
   * @param p Pointer to the process running on the core.
   */
  void TickExecution(Process* p) {
    if (p->GetState() == Process::RUNNING || p->GetState() == Process::READY) {
        if (p->GetCurrentInstructionCyclesLeft() <= 0) {
            p->ExecuteInstructions(coreNum);

            // Detach immediately if process transitioned to WAITING (SLEEP) or FINISHED
            if (p->GetState() == Process::WAITING || p->GetState() == Process::FINISHED) {
                currentProcess = nullptr;
                return;
            }

            p->SetCurrentInstructionCyclesLeft(ctx.delayPerExec);
        } else {
            p->DecrementInstructionCyclesLeft();
        }
    }
  }

  /**
   * @brief Performs a single CPU clock cycle of execution for the assigned process.
   */
  void RunCycle() {
    Process* p = currentProcess;
    if (p == nullptr) {
        return;   // Idle core
    }

    if (p->GetState() == Process::FINISHED) {
        currentProcess = nullptr;
        return;
    }

    if (TickPreemption(p)) {
        return;   // Process preempted and detached
    }

    TickExecution(p);
  }

  /**
   * @brief Worker thread's main execution loop representing a single CPU core.
   *
   * Ticks logical CPU cycles at a set interval and invokes core execution.
   *
   * @return A pointer to this Worker instance upon termination.
   */
  Worker* ThreadTask() {
    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(kTickDurationMs));

        std::lock_guard<std::mutex> lock(workerMutex);

        if (!running) break;

        RunCycle();
    }
    return this;
  }

private:
  int coreNum;
  AlgoContext ctx;
  prosched::Process *currentProcess = nullptr;
  std::thread workerThread;
  mutable std::mutex workerMutex;
  bool running = false;
  prosched::Process* preemptedProcess = nullptr;
};

} // namespace prosched