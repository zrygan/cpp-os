#pragma once

#include <bits/stdc++.h>
#include <stdio.h>
#include <vector>

#include "../config.h"

using namespace std;

enum class InstructionType { PRINT, DECLARE, ADD, SUBTRACT, SLEEP, FOR };

struct Instruction {
  InstructionType instructionType;
  vector<string> args;
  vector<Instruction> body; // for FOR instructions
  int repeatCount = 1;      // for FOR instructions
};

class Process {
public: 
  enum ProcessState {
    READY,
    RUNNING,
    WAITING,
    FINISHED
  };

  Process(std::string name, int id, int arrivalTick);
  void AddCommand(std::shared_ptr<Instruction> instruction);
  void ExecuteCurrentCommand();
  void AssignCore();

  bool IsFinished();
  int GetPID();
  std::string GetName();

private:
  std::string processName;
  int pid;
  std::vector<Instruction> instructions;
  int currentInstructionIndex = 0;
  int arrivalTick = 0;
  int coreAssigned = -1;
  ProcessState currentState;
  std::vector<std::string> logs;
  
};