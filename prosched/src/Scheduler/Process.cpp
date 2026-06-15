#include "Process.h"
#include <cstdlib>

Process::Process(std::string name, int id, int arrivalTick)
    : processName(processName), pid(pid), arrivalTick(arrivalTick) {}

void Process::AddCommand(std::shared_ptr<Instruction> instruction) {
  instructions.push_back(*instruction);
}

void Process::ExecuteCurrentCommand(){

}

void Process::MoveToNextLine() {

}


void Process::AssignCore() {

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