#include "View/View.h"

#ifndef CONTROLLER_H
#define CONTROLLER_H

class Controller {

private:
  View view;

public:
  void initialize();
  void run();
};

#endif