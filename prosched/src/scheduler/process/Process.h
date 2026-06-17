#pragma once

#include <stdio.h>
#include <vector>
#include <string>
#include <memory>

#include "config.h"
#include "src/commands/Command.h"

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

  /**
   * @brief adds a command to the process
   * 
   * Appends the specified randomized command to Process' commands vector
   * 
   * @param command Shared pointer to the command to be added
   * @return If adding a command is successful it returns the 
   * specific command, else unsuccessful return nullptr
   */
  std::shared_ptr<Command>* AddCommand(
    std::shared_ptr<Command> command
  );

  /**
   * @brief Executes the current randomized command assigned to the Process
   * 
   * disclaimer: function isnt implemented yet so this is how i think it would work atm
   * Takes the Process' commands vector and executes each command until
   * the commands vector is empty or if a command fails
   * 
   * @return If executing a command was successful the command is returned, else
   * unsuccessfull a nullptr is returned
   * @param currentInstructionIndex indicated index of command to be executed ?
   */
  Command* ExecuteCurrentCommand(
    int currentInstructionIndex
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

private:
  std::string processName;
  int pid;
  int coreNum;
  std::vector<Command> commands;
  int currentInstructionIndex = 0;
  int arrivalTick = 0;
  ProcessState currentState;
  
};