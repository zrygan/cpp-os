#include "Menu.h"
#include <iostream>

#ifdef _WIN32
#include <windows.h>
static HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
static void setColor(int color) {
  SetConsoleTextAttribute(hConsole, static_cast<WORD>(color));
}
#else
static void setColor(int color) {
  switch (color) {
  case 10:
    std::cout << "\033[32m";
    break;
  case 14:
    std::cout << "\033[33m";
    break;
  default:
    std::cout << "\033[0m";
    break;
  }
}
#endif

void welcome() {
  std::cout << R"(

  ______   _____   ____   _____   ______   _____  __     __
 / ____/  / ___/  / __ \ |  __ \ |  ____| / ____| \ \   / /
| |      | (___  | |  | || |__) || |__   | (___    \ \_/ / 
| |       \___ \ | |  | ||  ___/ |  __|   \___ \    \   /  
| |____   ____) || |__| || |     | |____  ____) |    | |   
 \_____| |_____/  \____/ |_|     |______||_____/     |_|   

)" << std::endl;
  setColor(10);
  std::cout << "Hello, welcome to the CSOPESY commandline!" << std::endl;
  setColor(14);
  std::cout << "Type 'exit' to quit, 'clear' to clear the screen." << std::endl;
  std::cout << "\n ** IMPORTANT: Type 'initialize' to load config and start "
               "system ** \n"
            << std::endl;
  setColor(7);
}
