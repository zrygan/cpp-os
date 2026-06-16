#pragma once

#include <stdio.h>
#include <thread>
#include <mutex>

#include "config.h"
#include "src/scheduler/process/Process.h"
#include "src/context.h"

class Worker {
public:
    /** 
     * \brief Creates a worker iassociated with a CPU core
     * 
     * @param coreNum the identifier of the CPU core assigned to this worker
     */
    Worker(int coreNum);

    /**
     * \brief Starts worker execution
     * 
     * Launches the worker thread and begins processing the assigned process
     */
    void Start();

    /**
     * \brief Stops worker execution
     * 
     * Signals the worker thread to terminate
     */
    void Stop();

    /**
     * \brief Assigns a process to the worker for execution
     * 
     * disclaimer: function isnt implemented yet so this is how i think it would work atm
     * 
     * The assigned process becomes the worker's current task and may
     * be executed when the worker is running.
     * 
     * @param p a pointer to the Process to assign
     */
    void AssignProcess(Process* p);

private:
    AlgoContext ctx;
    int coreNum;
    Process* currentProcess = nullptr;
    std::thread workerThread;
    std::mutex workerMutex;
    bool running = false;
    /**
     * \brief Worker thread's main execution loop
     * 
     */
    void ThreadTask();

};