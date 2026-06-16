#include <cstdlib>
#include <string>
#include <new>

#include "Process.h"
#include "src/commands/Command.h"

Process::Process(std::string processName, int pid, int arrivalTick)
    : processName(processName), pid(pid), arrivalTick(arrivalTick) {}

bool Process::AddCommand(std::shared_ptr<Command> command) {
  try {
    commands.push_back(*command);
    return true;
  } catch (const std::bad_alloc& e) {
    std::cerr << "Allocation failed: " << e.what();
    return false;
  }
}

bool Process::ExecuteCurrentCommand(int currentInstructionIndex){
  // run command in commands[currentInstructionIndex]
  // temp return
  return true;
}

bool Process::AssignCore(int coreNum) {
  //temp rturn
  return true;
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