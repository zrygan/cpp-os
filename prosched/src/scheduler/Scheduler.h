#pragma once

#include <mutex>
#include <queue>
#include <stdio.h>
#include <thread>
#include <vector>

#include "config.h"
#include "process/Process.h"
#include "src/context.h"
#include "src/scheduler/worker/Worker.h"
#include "src/Constants.hpp"

using namespace std;

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
    // ehhh
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
    }

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
    std::lock_guard<std::mutex> lock(schedulerMutex);
    try {
      processes.push_back(p);
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
            std::cout << "Core " << w->GetCoreNum() << " (idle)\n";
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
  prosched::Process *generateProcess(AlgoContext *ctx, int pid, int tick) {

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

    // std::cout << "\nFCFS Scheduler\n";
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
   *
   * detailed desc
   */
  void RoundRobin() {
    // @aaron future func for rr
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

      /*
        <RV @zrygan> ===========
        So cpuCycles % ctx.batchProcessFreq is required everywhere.

        Can we ensure in the the config loader or whereever the values
        for these are populated. We check that the values are
        valid/make sense. This can be a very easy pain point in the future.

        The same thing for limiting the number of processes to 10.


        ======

        Also again, separation of concerns. This is a function juggling
        a lot of things.

        @erin @aaron

        <RV @zrygan> ===========
      */
      if (generatingProcesses && cpuCycles % ctx.batchProcessFreq == 0) {

        // limit to 10 processes -> 10 txt files
        if (nextPID <= MAX_PROCESSES) {
          Process *p = generateProcess(&this->ctx, nextPID, cpuCycles);
          std::lock_guard<std::mutex> lock(schedulerMutex);
          processQueue.push(p);
          processes.push_back(p);

          nextPID++;
        } 
      }

      if (ctx.schedulerType == SchedulerType::FCFS) {
        FCFS();
      } else if (ctx.schedulerType == SchedulerType::RR) {
        RoundRobin();
      } else {
        std::cout << "Invalid scheduler type (check config)\n";
        return;
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }
};

} // namespace prosched