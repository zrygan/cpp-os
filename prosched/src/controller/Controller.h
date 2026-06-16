#include "view/View.h"
#include "context.h"
#include "src/scheduler/Scheduler.h"

#ifndef CONTROLLER_H
#define CONTROLLER_H

class Controller {

private:
  View view;
  AlgoContext ctx;
  Scheduler *scheduler;
  bool isInitialized;

public:
  void initialize();
  void run();
};

#endif