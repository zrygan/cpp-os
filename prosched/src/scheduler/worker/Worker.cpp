#include <stdio.h>

#include "Worker.h"

Worker::Worker(int coreNum) : coreNum(coreNum) {}

void Worker::Start() {
    running = true;
}

void Worker::Stop(){
    running = false;
}

void Worker::AssignProcess(Process* p) {
    currentProcess = p;
}

void Worker::ThreadTask() {
    currentProcess->ExecuteCurrentCommand(coreNum);
}