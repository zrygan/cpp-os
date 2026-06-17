#include <stdio.h>
#include <thread>
#include <vector>

#include "Scheduler.h"
#include "src/scheduler/process/Process.h"

Scheduler::Scheduler(AlgoContext ctx) : ctx(ctx) {}

bool Scheduler::Start(std::vector<thread> workerThreads){
    
}

void Scheduler::Stop(){

}

Scheduler* Scheduler::SchedulerLoop() {
    return this;
}

Process* Scheduler::AddProcess(Process* p){
    return p;
}

void Scheduler::PrintProcesses() {
    
}