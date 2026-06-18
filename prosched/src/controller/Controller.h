#include "context.h"
#include "src/scheduler/Scheduler.h"
#include "view/View.h"

#ifndef CONTROLLER_H
#define CONTROLLER_H

class Controller {

private:
  View view;
  AlgoContext ctx;
  prosched::Scheduler *scheduler;
  bool isInitialized;

public:
  AlgoContext initialize();
  void run();
};

#endif