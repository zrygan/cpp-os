#pragma once

#include <mutex>
#include <queue>
#include <random>
#include <stdio.h>
#include <thread>
#include <vector>

#include "Config.h"
#include "process/Process.h"
#include "src/Constants.hpp"
#include "src/Context.h"
#include "src/scheduler/worker/Worker.h"

namespace prosched {

class Scheduler {
public:
  /**
   * @brief Creates a scheduler instance
   *
   * Initializes the scheduler using the provided scheduling configuration
   *
   * @param ctx Scheduling configuration and algorithm settings
   */
  Scheduler(AlgoContext ctx) : ctx(ctx) {}

  /**
   * @brief Destroys the scheduler instance
   *
   * Stops the scheduler if running and cleans up all allocated resources
   */
  ~Scheduler() {
    if (running) {
      Stop();
    }
    for (Process *p : processes) {
      if (p && p->IsOwnedByScheduler()) {
        delete p;
      }
    }
    for (Worker *w : workers) {
      delete w;
    }
  }

  /**
   * @brief starts the main scheduler loop
   *
   * disclaimer: function isnt implemented yet so this is how i think it would
   * work atm Starting the scheduler starts a scheduler thread and calls on n
   * number of Workers (where n is the number of defined CPU cores) and starts
   * their individual thread tasks, once each thread is running the Start
   * function will then start the main scheduler loop and return a boolean value
   * turn running = true
   *
   * @return a boolean value, where true if all worker threads
   * have been joined, else false
   */
  bool Start() {
    if (running)
      return false;

    printSchedulerSpecs();

    running = true;
    generatingProcesses = true;

    for (int i = 0; i < this->ctx.num_cpu; i++) {
      Worker *w = new Worker(i, ctx);

      try {
        workers.push_back(w);
      } catch (const std::bad_alloc &e) {
        std::cerr << "Allocation failed: " << e.what();
        return false;
      }

      w->Start();
    }

    schedulerThread = std::thread(&Scheduler::SchedulerLoop, this);
    std::cout << "Scheduler started with " << this->ctx.num_cpu << " cores\n";
    return true;
  }

  /**
   * @brief Resumes process generation
   */
  void ResumeGenerating() { generatingProcesses = true; }

  /**
   * @brief Stops generating new dummy processes
   */
  void StopGenerating() {
    generatingProcesses = false;
    std::cout
        << "Stopped generating processes. Scheduler is still running.\n\n";
  }

  /**
   * @brief Stops the Scheduler from running
   *
   * running = false, stops all worker threads and the scheduler thread
   */
  void Stop() {
    running = false;
    generatingProcesses = false;

    if (schedulerThread.joinable()) {
      schedulerThread.join();
    }

    for (Worker *w : workers) {
      w->Stop();
      delete w;
    }
    workers.clear();

    // Empty the queue without deleting since destructor will handle processes
    while (!processQueue.empty()) {
      processQueue.pop();
    }

    std::cout << "Scheduler stopped\n\n";
  }

  /**
   * @brief Adds a Process to the processQueue
   *
   * Registers the specified process with the scheduler and places it into the
   * appropriate scheduling structure
   *
   * @param p A pointer to the process to be added
   * @return self if success, else returns none
   */
  prosched::Process *AddProcess(prosched::Process *p) {
    if (p == nullptr)
      return nullptr;
    std::lock_guard<std::mutex> lock(schedulerMutex);
    try {
      processes.push_back(p);
      processQueue.push(p);
      return p;
    } catch (const std::bad_alloc &e) {
      std::cerr << "Allocation failed: " << e.what();
      return nullptr;
    }
  }

  /**
   * @brief prints the current processes when "screen -ls" is called
   *
   * Outputs the current process list and their states
   */
  void PrintProcesses() {
    std::lock_guard<std::mutex> lock(schedulerMutex);

    double utilization = 0;
    int coresUsed = 0;
    int coresAvail = 0;

    for (Worker *w : workers) {
      if (w->IsBusy()) {
        coresUsed++;
      } else {
        coresAvail++;
      }
    }

    utilization = (static_cast<double>(coresUsed) / workers.size()) * 100.0;

    std::cout << "\nCPU utilization: " << utilization << "%\n";
    std::cout << "Cores used: " << coresUsed << "/" << workers.size() << "\n";
    std::cout << "Cores available: " << coresAvail << "/" << workers.size()
              << "\n";

    std::cout << "\n" << std::string(50, '-');
    std::cout << "\nRunning Processes: \n";

    for (Worker *w : workers) {
      Process *p = w->GetCurrentProcess();
      if (p != nullptr) {
        std::cout << p->GetName() << std::string(5, ' ')
                  << p->GetProcessTimeStart() << std::string(5, ' ')
                  << "Core: " << w->GetCoreNum() << std::string(5, ' ')
                  << p->GetCurrentInstructionIndex() << " / "
                  << p->GetTotalInstructions() << "\n";
      } else {
        std::cout << "Core " << w->GetCoreNum() << std::string(5, ' ')
                  << " (idle)\n";
      }
    }

    std::vector<Process *> finished;
    for (Process *p : processes) {
      if (p != nullptr && p->IsFinished()) {
        finished.push_back(p);
      }
    }

    int startIdx = std::max(0, (int)finished.size() - 10);

    std::cout << "\n\nFinished Processes: \n";
    for (int i = startIdx; i < (int)finished.size(); i++) {
      Process *p = finished[i];
      if (p != nullptr && p->IsFinished()) {
        std::cout << p->GetName() << std::string(5, ' ')
                  << p->GetProcessTimeStart() << std::string(5, ' ')
                  << "Finished" << std::string(5, ' ')
                  << p->GetCurrentInstructionIndex() << " / "
                  << p->GetTotalInstructions() << "\n";
      }
    }

    std::cout << std::string(50, '-') << "\n";
    std::cout << std::endl;
  }

  /**
   * @brief
   *
   * @param ctx
   * @param id
   * @param tick
   *
   * @return Process generated
   */
  prosched::Process *generateProcess(AlgoContext *ctx, int pid, int tick) {

    std::string name = "process" + std::to_string(nextPID);
    Process *p = new Process(name, pid, tick);
    p->SetOwnedByScheduler(true);
    Statement instruction;
    // std::vector<std::string> instructions = {"PRINT", "DECLARE", "ADD",
    //                                       "SUBTRACT", "SLEEP", "FOR"};

    // std::cout << p->GetName() << "\n";

    int commandAmount =
        this->ctx.min_ins + rand() % (ctx->max_ins - ctx->min_ins + 1);

    for (int i = 0; i < commandAmount; i++) {
      instruction = prosched::GetRandomStatement(name, 3);
      p->AddInstruction(instruction);
      // std::cout << p->GetName() << " added an instruction\n";
    }

    return p;
  }

  /**
   * @brief getter for running boolean
   *
   * @return boolean value if scheduler is running or not
   */
  bool IsRunning() { return running == true; }

  /**
   * @brief Handles periodic batch process generation.
   *
   * @param cpuCycles Current master clock tick cycle count.
   */
  void GenerateProcessesCycle(int cpuCycles) {
    if (generatingProcesses && cpuCycles % ctx.batch_process_frequency == 0) {
      Process *p = generateProcess(&this->ctx, nextPID, cpuCycles);
      std::lock_guard<std::mutex> lock(schedulerMutex);
      processQueue.push(p);
      processes.push_back(p);

      nextPID++;
    }
  }

  /**
   * @brief Gathers processes preempted by CPU workers and places them back in
   * the queue.
   */
  void CollectPreemptedCycle() {
    std::vector<Process *> preemptedList;
    for (Worker *w : workers) {
      Process *preempted = w->GetAndClearPreemptedProcess();
      if (preempted) {
        preemptedList.push_back(preempted);
      }
    }
    if (!preemptedList.empty()) {
      std::lock_guard<std::mutex> lock(schedulerMutex);
      for (Process *preempted : preemptedList) {
        processQueue.push(preempted);
      }
    }
  }

  /**
   * @brief Updates sleeping processes and re-queues them when their sleep ticks
   * expire.
   */
  void UpdateSleepingProcessesCycle() {
    std::lock_guard<std::mutex> lock(schedulerMutex);
    for (Process *p : processes) {
      if (p != nullptr && p->GetState() == ProcessState::WAITING) {
        p->DecrementSleepCycles();
        if (p->GetCyclesRemainingForSleep() <= 0) {
          if (p->GetCurrentInstructionIndex() >= p->GetTotalInstructions()) {
            p->SetState(ProcessState::FINISHED);
          } else {
            p->SetState(ProcessState::READY);
            processQueue.push(p);
          }
        }
      }
    }
  }

  /**
   * @brief The main scheduler control loop running in a dedicated thread.
   *
   * Drives the master scheduler clock and invokes sub-cycle operations.
   */
  void SchedulerLoop() {
    int cpuCycles = 0;

    while (running) {
      cpuCycles++;

      GenerateProcessesCycle(cpuCycles);

      if (ctx.schedulerType == SchedulerType::FCFS) {
        FCFS();
      } else if (ctx.schedulerType == SchedulerType::RR) {
        RoundRobin();
      }

      CollectPreemptedCycle();
      UpdateSleepingProcessesCycle();

      std::this_thread::sleep_for(std::chrono::milliseconds(kTickDurationMs));
    }
  }

private:
  AlgoContext ctx;
  std::thread schedulerThread;
  std::vector<prosched::Process *> processes;
  std::queue<prosched::Process *> processQueue;
  std::vector<Worker *> workers;
  std::mutex schedulerMutex;
  bool running = false;
  bool generatingProcesses = false;
  int nextPID = 1;

  /**
   * @brief converts the scheduler type enum to a string for printing
   *
   * @return a string representation of the scheduler type
   */
  std::string schedulerTypeToString() {
    switch (ctx.schedulerType) {
    case SchedulerType::FCFS:
      return "fcfs";
    case SchedulerType::RR:
      return "rr";
    default:
      return "unknown";
    }
  }

  /**
   * @brief prints the scheduler specs when "screen -ls" is called
   */
  void printSchedulerSpecs() {
    std::cout << "\n\nScheduler specs\n";
    std::cout << "\nNumber of cores: " << this->ctx.num_cpu;
    std::cout << "\nScheduler: " << schedulerTypeToString();
    std::cout << "\nBatch process freq: " << this->ctx.batch_process_frequency;
    std::cout << "\nMin-ins & Max-ins: " << this->ctx.min_ins << "-"
              << this->ctx.max_ins;
    std::cout << "\nDelay per execution: " << this->ctx.delay_per_execution;
    std::cout << "\nQuantum cycle: " << this->ctx.rr_quantum_cycles << "\n\n";
  }

  /**
   * @brief First-Come First-Serve Scheduler Algorithm
   *
   * A non-preemptive algorithm in which processes are attended to in
   * the order they arrive in the process queue. For each CPU worker
   * in the workers vector assign processes from the process queue to
   * a specific Worker to be executed.
   */
  void FCFS() {
    std::lock_guard<std::mutex> lock(schedulerMutex);
    for (Worker *w : workers) {
      if (!processQueue.empty() && !w->IsBusy()) {
        Process *p = processQueue.front();
        processQueue.pop();
        w->AssignProcess(p);
      }
    }
  }

  /**
   * @brief Round Robin Scheduler Algorithm
   *
   * A preemptive algorithm in which processes are given a fixed time slice (quantum).
   * Increments quantum used for running processes on cores and preempts them if
   * the quantum limit is reached. Then dispatches ready processes to available cores.
   */
  void RoundRobin() {
    // 1. Quantum preemption check
    for (Worker *w : workers) {
      if (w->CheckAndIncrementQuantum(ctx.rr_quantum_cycles)) {
        w->PreemptProcess();
      }
    }

    // 2. Dispatch ready processes
    std::lock_guard<std::mutex> lock(schedulerMutex);
    for (Worker *w : workers) {
      if (!processQueue.empty() && !w->IsBusy()) {
        Process *p = processQueue.front();
        processQueue.pop();
        w->AssignProcess(p);
      }
    }
  }
};

} // namespace prosched
