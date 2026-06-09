#include "task_manager.h"
#include "imgui.h"
#include <cstdio>

void TaskManager::display() {
}

void TaskManager::initialize(std::vector<ProcessDetail> inputVector) {
    this->pds = inputVector;
}
