#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H

#include <string>
#include <vector>

class TaskManager {
public:
  struct ProcessDetail {
    std::string processName;
    int cpu;
    int memory;
    int gpu;
    int disk;
    int network;
  };

private:
  std::vector<ProcessDetail> pds;

public:
  void display();
  void randomize();
  void initialize();
};

#endif