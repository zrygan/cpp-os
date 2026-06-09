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
  std::string name;
  int id;
  std::vector<Instruction> instructions;
  int currentInstructionIndex = 0;
  int arrivalTick = 0;
  int coreAssigned = -1;
  bool finished = false;
  std::vector<std::string> logs;

  Process(std::string name, int id, int arrivalTick);
};