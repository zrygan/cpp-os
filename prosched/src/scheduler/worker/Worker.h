#pragma once

#include <stdio.h>
#include <thread>
#include <mutex>

#include "config.h"
#include "scheduler/process/Process.h"
#include "context.h"

class Worker {
public:
    /** 
     * @brief Creates a worker associated with a CPU core
     * 
     * @param coreNum the identifier of the CPU core assigned to this worker
     * @param ctx Scheduling configuration
     */
    Worker(int coreNum, AlgoContext ctx);

    /**
     * @brief Starts worker execution
     * 
     * Launches the worker thread and begins processing the assigned process
     * @return true if start is successful
     */
    bool Start();

    /**
     * @brief Stops worker execution
     * 
     * Signals the worker thread to terminate
     * @return true if stop is successful
     */
    bool Stop();

    /**
     * @brief Assigns a process to the worker for execution
     * 
     * The assigned process becomes the worker's current task and may
     * be executed when the worker is running.
     * 
     * @param p a pointer to the Process to assign
     * @return the assigned process if successful, or nullptr if busy
     */
    Process* AssignProcess(Process* p);

    /**
     * @brief Checks if the worker is currently running
     * 
     * @return true if the worker thread is active
     */
    bool IsRunning() const;

    /**
     * @brief Checks if the worker is currently executing a process
     * 
     * @return true if a process is assigned and executing
     */
    bool IsBusy() const;

    /**
     * @brief Gets the core number associated with this worker
     * 
     * @return The CPU core identifier
     */
    int GetCoreNum() const;

    /**
     * @brief Gets the process currently assigned to this worker
     * 
     * @return A pointer to the current Process, or nullptr if idle
     */
    Process* GetCurrentProcess() const;

private:
    AlgoContext ctx;
    int coreNum;
    Process* currentProcess = nullptr;
    std::thread workerThread;
    mutable std::mutex workerMutex;
    bool running = false;

    /**
     * @brief Worker thread's main execution loop
     * 
     * @return self if successful once stopped
     */
    Worker* ThreadTask();
};