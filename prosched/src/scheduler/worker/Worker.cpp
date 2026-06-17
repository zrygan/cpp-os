#include <stdio.h>
#include <iostream>

#include "Worker.h"
#include <chrono>

Worker::Worker(int coreNum, AlgoContext ctx) : coreNum(coreNum), ctx(ctx), running(false), currentProcess(nullptr) {}

bool Worker::Start() {

    std::lock_guard<std::mutex> lock(workerMutex);

    if (running) {
        std::cout << "Worker on core " << coreNum << " is already running\n";
        return false;
    }
    running = true;

    workerThread = std::thread(&Worker::ThreadTask, this);
    return true;

}

bool Worker::Stop(){
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

Process* Worker::AssignProcess(Process* p) {
    std::lock_guard<std::mutex> lock(workerMutex);
    if (currentProcess != nullptr) {
        std::cout << "Worker on core " << coreNum << " is already assigned a process\n";
        return nullptr;
    }
    currentProcess = p;
    return currentProcess;
}

Worker* Worker::ThreadTask() {
    while (running) {
        Process* p = nullptr;
        {
             std::lock_guard<std::mutex> lock(workerMutex);
             p = currentProcess;
        }

        if (p != nullptr) {
            p->ExecuteCurrentCommand(coreNum);
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

bool Worker::IsRunning() const {
    return running;
}

bool Worker::IsBusy() const {
    return currentProcess != nullptr;
}

int Worker::GetCoreNum() const {
    return coreNum;
}

Process* Worker::GetCurrentProcess() const {
    std::lock_guard<std::mutex> lock(workerMutex);
    return currentProcess;
}