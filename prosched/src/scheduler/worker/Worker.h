#pragma once

#include <stdio.h>
#include <thread>
#include <mutex>

#include "config.h"
#include "scheduler/process/Process.h"
#include "context.h"

using namespace std;

namespace prosched {

    class Worker {
    public:
        /** 
         * @brief Creates a worker associated with a CPU core
         * 
         * @param coreNum the identifier of the CPU core assigned to this worker
         * @param ctx Scheduling configuration
         */
        Worker(int coreNum, AlgoContext ctx) : coreNum(coreNum), ctx(ctx), running(false), currentProcess(nullptr) {}

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

            if (workerThread.joinable()){
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
        prosched::Process* AssignProcess(prosched::Process* p) {
            std::lock_guard<std::mutex> lock(workerMutex);
            if (currentProcess != nullptr) {
                // std::cout << "Worker on core " << coreNum << " is already assigned a process\n";
                return nullptr;
            }
            currentProcess = p;
            if (p) p->AssignCore(coreNum);
            return p;
        }

        /**
         * @brief Checks if the worker is currently running
         * 
         * @return true if the worker thread is active
         */
        bool IsRunning() const {
            return running;
        }

        /**
         * @brief Checks if the worker is currently executing a process
         * 
         * @return true if a process is assigned and executing
         */
        bool IsBusy() const {
            return currentProcess != nullptr;
        }

        /**
         * @brief Gets the core number associated with this worker
         * 
         * @return The CPU core identifier
         */
        int GetCoreNum() const {
            return coreNum;
        }

        /**
         * @brief Gets the process currently assigned to this worker
         * 
         * @return A pointer to the current Process, or nullptr if idle
         */
        prosched::Process* GetCurrentProcess() const {
            std::lock_guard<std::mutex> lock(workerMutex);
            return currentProcess;
        }

    private:
        AlgoContext ctx;
        int coreNum;
        prosched::Process* currentProcess = nullptr;
        std::thread workerThread;
        mutable std::mutex workerMutex;
        bool running = false;

        /**
         * @brief Worker thread's main execution loop
         * 
         * @return self if successful once stopped
         */
        Worker* ThreadTask() {
            while (running) {
                prosched::Process* p = nullptr;
                {
                    std::lock_guard<std::mutex> lock(workerMutex);
                    p = currentProcess;
                }

                if (p != nullptr) {
                    p->ExecuteInstructions(coreNum);
                    // std::cout << "Worker on core " << coreNum << " is executing process " << currentProcess->GetName() << "\n";
                    
                    if (p->IsFinished()) {
                        std::lock_guard<std::mutex> lock(workerMutex);
                        currentProcess = nullptr; // Process finished, core is now free
                    }
                } else {
                    // std::cout << "Worker on core " << coreNum << " is idle\n";
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
            }
            return this;
        }
    };

}