#pragma once

#include <mutex>
#include <queue>
#include <stdio.h>
#include <thread>
#include <vector>

#include "src/Constants.hpp"
#include "config.h"
#include "process/Process.h"
#include "src/context.h"
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
    for (Process* p : processes) {
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

    for (int i = 0; i < this->ctx.numCpu; i++) {
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
    std::cout << "Scheduler started with " << this->ctx.numCpu << " cores\n";
    return true;
  }

  /**
   * @brief Resumes process generation
   */
  void ResumeGenerating() {
    generatingProcesses = true;
  }

  /**
   * @brief Stops generating new dummy processes
   */
  void StopGenerating() {
    generatingProcesses = false;
    std::cout << "Stopped generating processes. Scheduler is still running.\n\n";
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
    if (p == nullptr) return nullptr;
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

    double utilization =0 ;
    int coresUsed = 0;
    int coresAvail = 0;

    for (Worker* w : workers) {
      if(w->IsBusy()) {
        coresUsed++;
      } else {
        coresAvail++;
      }
    }

    utilization = (static_cast<double>(coresUsed)/workers.size())*100.0;

    std::cout << "\nCPU utilization: " << utilization << "%\n";
    std::cout << "Cores used: " << coresUsed << "/" << workers.size() << "\n";
    std::cout << "Cores available: " << coresAvail << "/" << workers.size() << "\n";

    std::cout << "\n" << std::string(50, '-');
    std::cout << "\nRunning Processes: \n";

    for (Worker* w : workers) {
        Process* p = w->GetCurrentProcess();
        if(p != nullptr) {
            std::cout << 
                p->GetName() << 
                std::string(5, ' ') <<
                p->GetProcessTimeStart() <<
                std::string(5, ' ') <<
                "Core: " << w->GetCoreNum() <<
                std::string(5, ' ') << 
                p->GetCurrentInstructionIndex() << " / " << p->GetTotalInstructions() <<
                "\n";
        } else {
            std::cout << "Core "  << w->GetCoreNum() << std::string(5, ' ') << " (idle)\n";
        }
    }

    std::vector<Process*> finished;
    for (Process* p : processes) {
        if (p != nullptr && p->IsFinished()) {
            finished.push_back(p);
        }
    }

    int startIdx = std::max(0, (int)finished.size() - 10);

    std::cout << "\n\nFinished Processes: \n";
    for (int i = startIdx; i < (int)finished.size(); i++) {
      Process* p = finished[i];
      if (p != nullptr && p->IsFinished()) {
        std::cout << p->GetName() << 
            std::string(5, ' ') <<
            p->GetProcessTimeStart() <<
            std::string(5, ' ') <<
            "Finished" <<
            std::string(5, ' ') <<
            p->GetCurrentInstructionIndex() << " / " << p->GetTotalInstructions() <<
            "\n";
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
  prosched::Process *generateProcess(AlgoContext */*ctx*/, int pid, int tick) {

    std::string name = "process" + std::to_string(nextPID);
    Process *p = new Process(name, pid, tick);
    p->SetOwnedByScheduler(true);

    // std::cout << p->GetName() << "\n";

    // uncomment this for future
    // int commandAmount = this->ctx.minIns + rand() % (this->ctx.maxIns -
    // this->ctx.minIns + 1);

    // fcfs specs say only 100

    int commandAmount = NUM_PRINT_INSTRUCTIONS;

    // only prints for now
    for (int i = 0; i < commandAmount; i++) {
      p->AddInstruction("PRINT(\"Hello world from " + name + "!\")");
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
    
  }

  /**
   * @brief Gathers processes preempted by CPU workers and places them back in the queue.
   */
  void CollectPreemptedCycle() {
    
  }

  /**
   * @brief Updates sleeping processes and re-queues them when their sleep ticks expire.
   */
  void UpdateSleepingProcessesCycle() {
    
  }

  /**
   * @brief Dispatches ready processes to idle worker CPU cores.
   */
  void DispatchProcessesCycle() {
    
  }

  
  /**
   * @brief the main scheduler loop
   *
   * Continuously selects and dispatches processes according to
   * the configured scheduling algorithm until the scheduler is stopped.
   *
   */
  void SchedulerLoop() {
    int cpuCycles = 0;

    while (running) {
      cpuCycles++;

      // 1. Generate Processes
      if (generatingProcesses && cpuCycles % ctx.batchProcessFreq == 0) {
        if (nextPID <= MAX_PROCESSES) {
          Process *p = generateProcess(&this->ctx, nextPID, cpuCycles);
          std::lock_guard<std::mutex> lock(schedulerMutex);
          processQueue.push(p);
          processes.push_back(p);

          nextPID++;
        } 
      }

      // 2. Collect preempted processes and re‑queue them
      for (Worker* w : workers) {
          Process* preempted = w->GetAndClearPreemptedProcess();
          if (preempted) {
              std::lock_guard<std::mutex> lock(schedulerMutex);
              processQueue.push(preempted);
          }
      }

      // 2.5. Update sleeping processes and re-queue them upon wake-up
      {
          std::lock_guard<std::mutex> lock(schedulerMutex);
          for (Process* p : processes) {
              if (p != nullptr && p->GetState() == Process::WAITING) {
                  p->DecrementSleepCycles();
                  if (p->GetCyclesRemainingForSleep() <= 0) {
                      if (p->GetCurrentInstructionIndex() >= p->GetTotalInstructions()) {
                          p->SetState(Process::FINISHED);
                      } else {
                          p->SetState(Process::READY);
                          processQueue.push(p);
                      }
                  }
              }
          }
      }

      // 3. Dispatch processes to idle workers
      {
        std::lock_guard<std::mutex> lock(schedulerMutex);
        for (Worker* w : workers) {
          if (!processQueue.empty() && !w->IsBusy()) {
            Process* p = processQueue.front();
            processQueue.pop();
            w->AssignProcess(p);
          }
        }
      }

      // 4. Tick wait
      std::this_thread::sleep_for(std::chrono::milliseconds(TICK_DURATION_MS));
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
    std::cout << "\nNumber of cores: " << this->ctx.numCpu;
    std::cout << "\nScheduler: " << schedulerTypeToString();
    std::cout << "\nBatch process freq: " << this->ctx.batchProcessFreq;
    std::cout << "\nMin-ins & Max-ins: " << this->ctx.minIns << "-"
              << this->ctx.maxIns;
    std::cout << "\nDelay per execution: " << this->ctx.delayPerExec;
    std::cout << "\nQuantum cycle: " << this->ctx.quantumCycles << "\n\n";
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
   * @brief Round Robin Scheduler
   */
  void RoundRobin() {
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