#pragma once

#include <stdio.h>
#include <vector>
#include <string>

using namespace std;

enum class CommandType { PRINT, DECLARE, ADD, SUBTRACT, SLEEP, FOR };

class Command {
private:
    CommandType commandName;
    std::vector<string> args;

    // for 'FOR' instructions
    std::vector<CommandType> commandList;
    int repeatCount = 1;

    void CommandPrint();
    void CommandDeclare();
    int CommandAdd();
    int CommandSubtract();
    void CommandSleep();
    void CommandFor();

public:
    void ProcessCommand(std::string commandName);
};