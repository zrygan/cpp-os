#include "View.h"
#include "page/Menu.h"
#include <cstdlib>

void View::displayMenu() { welcome(); }

void View::displayClear() {
  system("cls");
#else
  system("clear");
}