#pragma once

#include <stdio.h>
#include <vector>
#include <string>
#include <memory>

#include "config.h"
#include "../commands/Command.h"

using namespace std;

class Process {
public: 
  enum ProcessState {
    READY,
    RUNNING,
    WAITING,
    FINISHED
  };

  Process(std::string name, int id, int arrivalTick);
  void AddCommand(std::shared_ptr<Command> command);
  void ExecuteCurrentCommand(int coreNum);
  void AssignCore();

  bool IsFinished();
  int GetPID();
  std::string GetName();

private:
  std::string processName;
  int pid;
  std::vector<Command> commands;
  int currentInstructionIndex = 0;
  int arrivalTick = 0;
  ProcessState currentState;
  
};