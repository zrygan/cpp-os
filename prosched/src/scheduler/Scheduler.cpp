#include <stdio.h>
#include <iostream>
#include <thread>
#include <vector>
#include <new>
#include <optional>
#include <chrono>

#include "Scheduler.h"
#include "src/scheduler/process/Process.h"
#include "src/scheduler/worker/Worker.h"

Scheduler::Scheduler(AlgoContext ctx) : ctx(ctx) {}

bool Scheduler::Start(){
    if (running) return false;

    // testing print
    std::cout << "\n\nScheduler specs\n";
    std::cout << "\nNumber of cores: " << this->ctx.numCpu;
    std::cout << "\nScheduler: " << this->ctx.schedulerType;
    std::cout << "\nBatch process freq: " << this->ctx.batchProcessFreq;
    std::cout << "\nMin-ins & Max-ins: " << this->ctx.minIns << "-" << this->ctx.maxIns;
    std::cout << "\nDelay per execution: " << this->ctx.delayPerExec;
    std::cout << "\nQuantum cycle: " << this->ctx.quantumCycles << "\n\n";
    
    running = true;

    // @aaron just edit this if something changes with how Worker functions 
    for (int i = 0; i <= this->ctx.numCpu; i++){
        Worker* w = new Worker(i, ctx);

        try{
            workers.push_back(w);
        } catch (const std::bad_alloc& e) {
            std::cerr << "Allocation failed: " << e.what();
            return false;
        }

        w->Start();
    }

    schedulerThread = std::thread(&Scheduler::SchedulerLoop, this);
    std::cout << "Scheduler started with " << this->ctx.numCpu << " cores\n";
    return true;
}

void Scheduler::Stop(){
    running = false;

    if (schedulerThread.joinable()){
        schedulerThread.join();
    }

    for (Worker* w : workers) {
        w->Stop();
    }

    std::cout << "Scheduler stopped\n";
}

Scheduler* Scheduler::SchedulerLoop() {
    while(running) {
        std::lock_guard<std::mutex> lock(schedulerMutex);

        if (this->ctx.schedulerType == "fcfs"){
            // add fcfs algorithm here
        } else if (this->ctx.schedulerType == "rr"){

        } else {
            std::cout << this->ctx.schedulerType << " is not a valid scheduler type\n";
            return nullptr;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return this;
}

Process* Scheduler::AddProcess(Process* p){
    try {
        processes.push_back(*p);
        return p;
    } catch (const std::bad_alloc& e) {
        std::cerr << "Allocation failed: " << e.what();
        return nullptr;
    }
}

void Scheduler::PrintProcesses() { //screen -ls

}

bool Scheduler::IsRunning() {
    return running;
}