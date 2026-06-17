#include <stdio.h>

#include "Worker.h"

Worker::Worker(int coreNum) : coreNum(coreNum) {}

bool Worker::Start() {
    running = true;
    return running;
}

void Worker::Stop(){
    running = false;
}

Process* Worker::AssignProcess(Process* p) {
    currentProcess = p;
    return currentProcess;
}

Worker* Worker::ThreadTask() {
    currentProcess->ExecuteCurrentCommand(coreNum);
    return this;
}