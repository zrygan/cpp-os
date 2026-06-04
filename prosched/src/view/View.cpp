#include "View.h"
#include "page/Menu.h"
#include <cstdlib>

void View::displayMenu() { welcome(); }

void View::displayClear() {
#ifdef _WIN32
  system("cls");
#else
  system("clear");
#endif
}