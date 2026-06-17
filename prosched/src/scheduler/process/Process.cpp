#include <cstdlib>
#include <string>
#include <new>
#include <optional>

#include "Process.h"
#include "src/commands/Command.h"

Process::Process(std::string processName, int pid, int arrivalTick)
    : processName(processName), pid(pid), arrivalTick(arrivalTick) {}

std::shared_ptr<Command>* Process::AddCommand(std::shared_ptr<Command> command) {
  try {
    commands.push_back(*command);
    return &command;
  } catch (const std::bad_alloc& e) {
    std::cerr << "Allocation failed: " << e.what();
    return nullptr;
  }
}

Command* Process::ExecuteCurrentCommand(int currentInstructionIndex){
  // run command in commands[currentInstructionIndex]
  // temp return
  return &commands.at(currentInstructionIndex);
}

int Process::AssignCore(int coreNum) {
  //temp rturn
  if (coreNum) {
    return coreNum;
  } else {
    return -1;
  }
}

bool Process::IsFinished() {
  if (currentState == FINISHED){
    return true;
  }
  return false;
}

int Process::GetPID() {
  return pid;
}

std::string Process::GetName() {
  return processName;
}

Process *generateProcess(ConfigStruct *config, int id, int tick) {

  std::string name = std::to_string(id) + std::to_string(tick);
  Process *p = new Process(name, id, tick);

  return p;
}