#pragma once

#include <stdio.h>
#include <vector>
#include <queue>
#include <mutex>
#include <thread>

#include "config.h"
#include "process/Process.h"
#include "../context.h"

class Scheduler {
public:
    Scheduler(AlgoContext ctx);

    /**
     * \brief starts the main scheduler loop
     * 
     * disclaimer: function isnt implemented yet so this is how i think it would work atm
     * Starting the scheduler starts a scheduler thread and calls on n number of Workers 
     * (where n is the number of defined CPU cores) and starts their individual thread tasks, 
     * once each thread is running the Start function will then start the main scheduler loop 
     * and return a boolean value
     * turn running = true
     * 
     * @param workerThreads is a vector of CPU worker threads
     * to be used by the main scheduler loop
     * @return a boolean value, where true if all worker threads
     * have been joined, else false
     */
    bool Start(
        std::vector<thread> workerThreads
    );

    /**
     * \brief Stops the Scheduler from running
     * 
     * running = false, stops all worker threads and the scheduler thread
     */
    void Stop();

    /**
     * \brief Adds a Process to the processQueue ?
     * 
     * detailed description
     * @param p is the process to be added
     * @return self if success, else returns none
     */
    Process AddProcess(Process *p);

    /**
     * \brief prints the current processes when "screen -ls" is called
     */
    void PrintProcesses();

private:
    AlgoContext ctx;
    std::thread schedulerThread;
    std::vector<Process> processes;
    std::queue<Process> processQueue;
    std::vector<std::thread> workerThreads;
    std::mutex schedulerMutex;
    bool running = false;

    /** 
     * \brief the main scheduler loop
     * 
     * 
    */
    void SchedulerLoop();
};