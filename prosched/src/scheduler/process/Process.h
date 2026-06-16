#pragma once

#include <stdio.h>
#include <vector>
#include <string>
#include <memory>

#include "config.h"
#include "../../commands/Command.h"

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
   * \brief adds a randomized command to Process' commands vector
   * 
   * @return a boolean value determing if the new command has been successfully
   * appended to the Process' commands vec
   * @param a pointer to a command function
   */
  bool AddCommand(
    std::shared_ptr<Command> command
  );

  /**
   * \brief Executes a randomized command inside Process
   * 
   * disclaimer: function isnt implemented yet so this is how i think it would work atm
   * Takes the Process' commands vector and executes each command until
   * the commands vector is empty or if a 
   * 
   * @return a boolean value returns true if all commands run successfully,
   * returns false if a command is unsuccessful
   * @param coreNum for PRINT command?
   */
  bool ExecuteCurrentCommand(
    int coreNum
  );

  /**
   * \brief Assigns a CPU core to the Process
   * 
   * disclaimer: function isnt implemented yet so this is how i think it would work atm
   * detailed description
   * 
   * @return a boolean value that determines if assigning a core was successful
   * @param coreNum indicates the CPU core to be assigned
   */
  bool AssignCore(int coreNum);

  bool IsFinished();
  int GetPID();
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