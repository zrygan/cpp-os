#include <cstdlib>
#include <string>
#include <new>
#include <optional>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>

#include "Process.h"
#include "src/commands/Command.h"
#include "src/commands/interpreter.hpp"

namespace prosched {

  Process::Process(std::string processName, int pid, int arrivalTick)
      : processName(processName), pid(pid), arrivalTick(arrivalTick) {}

  std::string Process::AddInstruction(std::string instruction) {
    try {
      auto stmts = interpreter.parse(instruction);
      statements.insert(statements.end(), stmts.begin(), stmts.end());
      return instruction;
    } catch (const std::bad_alloc& e) {
      std::cerr << "Allocation failed: " << e.what();
      return nullptr;
    }
  }

  // this is incomplete
  std::string Process::ExecuteCurrentCommand(int coreNum){
  
    currentState = RUNNING;

    if (currentInstructionIndex >= statements.size()) {
        currentState = FINISHED;
        return;
    }

    interpreter.executeStatements(
        { statements[currentInstructionIndex] }
    );
    currentInstructionIndex++;

    return ;
  }

  int Process::AssignCore(int coreNum) {
    if (coreNum) {
      this->coreNum = coreNum;
      return coreNum;
    } else {
      return -1;
    }
  }

  bool Process::IsFinished() {
    return currentState == FINISHED;
  }

  int Process::GetPID() {
    return pid;
  }

  std::string Process::GetName() {
    return processName;
  }

  std::string Process::GetTimestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm* tm = std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(tm, "(%m/%d/%Y %I:%M:%S%p)");
    return oss.str();
  }
} // namspace prosched