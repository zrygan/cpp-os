#include "Process.h"
#include <cstdlib>

Process::Process(std::string name, int id, int arrivalTick)
    : name(name), id(id), arrivalTick(arrivalTick) {}

Process* generateProcess(ConfigStruct* config, int id, int tick) {

    std::string name = std::to_string(id) + std::to_string(tick);
    Process* p = new Process(name, id, tick);

    return p;
}