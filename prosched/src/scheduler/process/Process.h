#pragma once

#include <stdio.h>
#include <vector>
#include <string>
#include <memory>

#include "config.h"
#include "src/commands/interpreter.hpp"

using namespace std;

class Process {
public: 
  enum ProcessState {
    READY,
    RUNNING,
    WAITING,
    FINISHED
  };

  Process(std::string processName, int id, int arrivalTick);

  // @stephen @aaron this was prev std::shared_ptr<Command> AddCommand(std::shared_ptr<Command> command);
  /**
   * @brief adds a command to the process
   * 
   * Appends the specified randomized command to Process' commands vector
   * 
   * @param instruction 
   * @return If adding a command is successful it returns the 
   * specific command, else unsuccessful return nullptr
   */
  std::string AddInstruction(std::string instruction);

  // @stephen @aaron this was previously "Command* ExecuteCurrentCommand(int currentInstructionIndex)"
  /**
   * @brief Executes all commands in statements
   * 
   * disclaimer: function isnt implemented yet so this is how i think it would work atm
   * Takes the statements vector and executes each command until
   * statements is empty 
   * 
   * @param coreNum
   * @return If executing a command was successful the command is returned, else
   * unsuccessfull a nullptr is returned 
   */
  std::string ExecuteInstructions (
    int coreNum
  );

  /**
   * @brief Assigns a CPU core to the Process
   * 
   * disclaimer: function isnt implemented yet so this is how i think it would work atm
   * Stores the identifier of the CPU core responsible for executing this process (?)
   * 
   * @param coreNum indicates the CPU core to be assigned
   * @return coreNum is returned if the assignment was successful; otherwise -1
   */
  int AssignCore(
    int coreNum
  );

  /**
   * @brief Checks if the Process has finished executing all its commands
   * 
   * A process is considered finished when all commands have been executed
   * 
   * @return true if the process is finished; otherwise false.
   */
  bool IsFinished();

  /**
   * @brief Gets the Process ID
   * 
   * @return the Process ID
   */
  int GetPID();

  /**
   * @brief Gets the Process name
   * 
   * @return the Process name
   */
  std::string GetName();

  /**
   * @brief Gets the process logs
   * 
   * @return vector of logs
   */
  std::vector<std::string> GetLogs();

private:
  std::string processName;
  int pid;
  int coreNum;
  int currentInstructionIndex = 0;
  int arrivalTick = 0;
  ProcessState currentState = READY;
  std::vector<std::string> logs;

  prosched::Interpreter interpreter;
  std::vector<prosched::Statement> statements;

  /**
   * @brief gets the current timestamp of a process
   * 
   * @return the timestamp string
   */
  std::string GetTimestamp();
  
};