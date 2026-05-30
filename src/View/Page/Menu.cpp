#include "Menu.h"
#include <iostream>
#include <windows.h>
using namespace std;

HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

void welcome() {
    std::cout << R"(

  ______   _____   ____   _____   ______   _____  __     __
 / ____/  / ___/  / __ \ |  __ \ |  ____| / ____| \ \   / /
| |      | (___  | |  | || |__) || |__   | (___    \ \_/ / 
| |       \___ \ | |  | ||  ___/ |  __|   \___ \    \   /  
| |____   ____) || |__| || |     | |____  ____) |    | |   
 \_____| |_____/  \____/ |_|     |______||_____/     |_|   

)" << std::endl;
SetConsoleTextAttribute(hConsole, 10);
std::cout << "Hello, welcome to the CSOPESY commandline!" << std::endl;
SetConsoleTextAttribute(hConsole, 14);
std::cout << "Type 'exit' to quit, 'clear' to clear the screen." << std::endl;
std::cout << "\n ** IMPORTANT: Type 'initialize' to load config and start system ** \n" << std::endl;
SetConsoleTextAttribute(hConsole, 7);
}
