#include "View.h"
#include "page/Menu.h"
#include <cstdlib>

/**
 * @brief Renders the main menu by calling Welcome() from the Menu page.
 */
void View::DisplayMenu() { Welcome(); }

/**
 * @brief Clears the terminal screen using a system call.
 */
void View::DisplayClear() { system("cls"); }