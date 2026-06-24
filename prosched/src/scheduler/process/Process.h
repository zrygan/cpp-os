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

  /**
   * @brief Executes the current instruction of the process on a CPU core.
   *
   * Executes one parsed Statement instruction. If it is the first statement,
   * sets the process's StartTime. Handles SLEEP operations by moving the process
   * to WAITING state and detaching from the core.
   *
   * @param coreNum The index of the assigned CPU core executing the process.
   * @return A vector containing all statements of the process.
   */
  std::vector<prosched::Statement> ExecuteInstructions(int coreNum) {

    currentState = RUNNING;

    if (currentInstructionIndex >= (int)statements.size()) {
      currentState = FINISHED;
      return {};
    }

    const Statement &stmt = statements[currentInstructionIndex];
    if (stmt.keyword == Keyword::SLEEP) {
      currentState = WAITING;
      if (!stmt.args.empty()) {
        try {
            cyclesRemainingForSleep = std::stoi(stmt.args[0]);
        }
        catch (const std::invalid_argument&) {
            cyclesRemainingForSleep = 0;
        }
        catch (const std::out_of_range&) {
            cyclesRemainingForSleep = 0;
        }
      } else {
        cyclesRemainingForSleep = 0;
      }
      currentInstructionIndex++;

      logs.push_back(GetTimestamp() + " Core:" + std::to_string(coreNum) +
                     " \"SLEEP " + (stmt.args.empty() ? "0" : stmt.args[0]) + "\"");

      if (currentInstructionIndex >= (int)statements.size() && cyclesRemainingForSleep == 0) {
        currentState = FINISHED;
      }
      return statements;
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

  /**
   * @brief Gets the current state of the process
   *
   * @return the current ProcessState
   */
  ProcessState GetState() const { return currentState; }

  /**
   * @brief Sets the current state of the process
   *
   * @param state the ProcessState to set
   */
  void SetState(ProcessState state) { currentState = state; }

  /**
   * @brief Gets the number of cycles remaining for the process to sleep
   *
   * @return the number of cycles remaining for sleep
   */
  int GetCyclesRemainingForSleep() const { return cyclesRemainingForSleep; }

  /**
   * @brief Decrements the number of cycles remaining for the process to sleep
   *
   * This function reduces the cyclesRemainingForSleep by one, ensuring it does
   * not go below zero.
   */
  void DecrementSleepCycles() { if (cyclesRemainingForSleep > 0) cyclesRemainingForSleep--; }

  /**
   * @brief Gets the number of cycles left for the current instruction
   *
   * @return the number of cycles left for the current instruction
   */
  int GetCurrentInstructionCyclesLeft() const { return currentInstructionCyclesLeft; }
  
  /**
   * @brief Sets the number of cycles left for the current instruction
   *
   * @param numCycles the number of cycles left for the current instruction
   */
  void SetCurrentInstructionCyclesLeft(int numCycles) { currentInstructionCyclesLeft = numCycles; }
  
  /**
   * @brief Decrements the number of cycles left for the current instruction
   *
   * This function reduces the currentInstructionCyclesLeft by one, ensuring it does
   * not go below zero.
   */
  void DecrementInstructionCyclesLeft() { if (currentInstructionCyclesLeft > 0) --currentInstructionCyclesLeft; }

  /**
   * @brief Gets the number of cycles used in the current quantum
   *
   * @return the number of cycles used in the current quantum
   */
  int GetQuantumUsed() const { return quantumUsed; }
  
  /**
   * @brief Increments the number of cycles used in the current quantum
   */
  void IncrementQuantumUsed() { ++quantumUsed; }
  
  /**
   * @brief Resets the number of cycles used in the current quantum to zero
   */
  void ResetQuantumUsed() { quantumUsed = 0; }


  
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

  int cyclesRemainingForSleep = 0;
  int currentInstructionCyclesLeft = 0;
  int quantumUsed = 0;


  prosched::Interpreter interpreter;
  std::vector<prosched::Statement> statements;

   /**
   * @brief Generates a formatted time string representing the current system clock.
   *
   * This is a utility function used to timestamp log messages. It does not alter
   * the process's internal start time.
   *
   * @return A string containing the formatted timestamp, e.g. "(06/24/2026 02:15:30PM)"
   */
  std::string GetTimestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm *tm = std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(tm, "(%m/%d/%Y %I:%M:%S%p)");
    return oss.str();
  }
};

} // namespace prosched