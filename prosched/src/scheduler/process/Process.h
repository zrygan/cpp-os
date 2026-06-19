#pragma once

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <memory>
#include <new>
#include <optional>
#include <sstream>
#include <stdio.h>
#include <string>
#include <vector>

#include "config.h"
#include "src/commands/Interpreter.hpp"

namespace prosched {

class Process {
public:
  enum ProcessState { READY, RUNNING, WAITING, FINISHED };

  Process(std::string processName, int pid, int arrivalTick)
      : processName(processName), pid(pid), arrivalTick(arrivalTick) {}

  // @stephen @aaron this was prev std::shared_ptr<Command>
  // AddCommand(std::shared_ptr<Command> command);
  /**
   * @brief adds a command to the process
   *
   * Appends the specified randomized command to Process' statements vector
   *
   * @param instruction
   * @return If adding a command is successful it returns the
   * specific command, else unsuccessful return nullptr
   */
  std::string AddInstruction(std::string instruction) {
    try {
      std::vector<Statement> stmts = interpreter.parse(instruction);
      statements.insert(statements.end(), stmts.begin(), stmts.end());
      return instruction;
    } catch (const std::bad_alloc &e) {
      std::cerr << "Allocation failed: " << e.what();
      return "";
    }
  }

  // @stephen @aaron this was previously "Command* ExecuteCurrentCommand(int
  // currentInstructionIndex) roger
  /**
   * @brief Executes all commands in statements
   *
   * disclaimer: function isnt implemented yet so this is how i think it would
   * work atm Takes the statements vector and executes each command until
   * statements is empty
   *
   * @param coreNum
   * @return If executing the instructions were successful the hmmm, else
   * unsuccessfull a nullptr is returned
   */
  std::vector<prosched::Statement> ExecuteInstructions(int coreNum) {

    currentState = RUNNING;

    if (currentInstructionIndex >= statements.size()) {
      currentState = FINISHED;
      return {};
    }

    interpreter.executeStatements({statements[currentInstructionIndex]});
    currentInstructionIndex++;

    auto output = interpreter.flushBuffer();
    for (const auto &line : output) {
      logs.push_back(GetTimestamp() + " Core:" + std::to_string(coreNum) +
                     " \"" + line + "\"");
    }

    if (currentInstructionIndex >= (int)statements.size()) {
      currentState = FINISHED;

      // remove for full proj
      SaveLogsToFile();
    }

    return statements;
  }

  /**
   * @brief Assigns a CPU core to the Process
   *
   * Stores the identifier of the CPU core responsible for executing this
   * process
   *
   * @param coreNum indicates the CPU core to be assigned
   * @return coreNum is returned if the assignment was successful; otherwise -1
   */
  int AssignCore(int coreNum) {
    if (coreNum >= 0) {
      this->coreNum = coreNum;
      return coreNum;
    } else {
      return -1;
    }
  }

  /**
   * @brief saves the logs vector into a txt file
   *
   * ngl idk if this should be here or insoide the executePrint() in
   * intepreter.hpp
   */
  void SaveLogsToFile() {
    std::filesystem::create_directory("logs");
    std::string filename = "logs/" + processName + ".txt";

    std::ofstream outFile(filename);
    if (outFile.is_open()) {
      outFile << "Process name: " << processName << "\n";
      for (const auto &log : logs) {
        outFile << log << "\n";
      }

      outFile.close();
    } else {
      std::cerr << "Error: Could not make log file for " << processName << "\n";
    }
  }

  /**
   * @brief Checks if the Process has finished executing all its commands
   *
   * A process is considered finished when all commands have been executed
   *
   * @return true if the process is finished; otherwise false.
   */
  bool IsFinished() { return currentState == FINISHED; }

  /**
   * @brief Gets the Process ID
   *
   * @return the Process ID
   */
  int GetPID() { return pid; }

  /**
   * @brief Gets the Process name
   *
   * @return the Process name
   */
  std::string GetName() { return processName; }

  /**
   * @brief Gets the assigned CPU core number
   *
   * @return the assigned CPU core number
   */
  int GetCoreNum() { return coreNum; }

  /**
   * @brief Gets the process logs
   *
   * @return vector of logs
   */
  std::vector<std::string> GetLogs() { return logs; }

  /**
   * @brief Gets the started time of a Process
   * 
   * @return the formatted time date string
   */
  std::string GetProcessTimeStart() { return StartTime; }

  /**
   * @brief gets the current index of an instruction being executed
   * 
   * for the screen -ls progress count
   * 
   * @return the current instruction index
   */
  int GetCurrentInstructionIndex() { return currentInstructionIndex; }

  /**
   * @brief get total number of statements/ instrcutions
   * 
   * for screen -ls progress count
   * 
   * @return total number of instrcutions in a process
   */
  int GetTotalInstructions() { return (int)statements.size(); }

  /**
   * @brief gets the core assigned
   * 
   * @return coreNum assigned
   */
  int GetAssignedCore() { return coreNum; }

  /**
   * @brief Sets whether the process is owned by the scheduler
   *
   * @param owned boolean value indicating ownership
   */
  void SetOwnedByScheduler(bool owned) { ownedByScheduler = owned; }

  /**
   * @brief Checks if the process is owned by the scheduler
   *
   * @return true if owned by the scheduler; otherwise false
   */
  bool IsOwnedByScheduler() const { return ownedByScheduler; }

  
private:
  std::string processName;
  int pid;
  int coreNum;
  int currentInstructionIndex = 0;
  int arrivalTick = 0;
  ProcessState currentState = READY;
  std::vector<std::string> logs;
  bool ownedByScheduler = false;
  std::string StartTime;

  prosched::Interpreter interpreter;
  std::vector<prosched::Statement> statements;

  /**
   * @brief gets the current timestamp of a process
   *
   * @return the timestamp string
   */
  std::string GetTimestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm *tm = std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(tm, "(%m/%d/%Y %I:%M:%S%p)");
    StartTime = oss.str();
    return StartTime;
  }
};

} // namespace prosched