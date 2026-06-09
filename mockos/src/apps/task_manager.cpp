#include "task_manager.h"
#include "imgui.h"
#include <algorithm>
#include <cstdio>
#include <random>

using PD = TaskManager::ProcessDetail;
static int PD::*const resources[] = {
    &PD::cpu, &PD::memory, &PD::gpu, &PD::disk, &PD::network,
};

static std::vector<PD> inputVector = {
    {"System", 80, 60, 50, 30, 20},        {"Explorer", 45, 70, 30, 10, 5},
    {"Chrome", 12, 40, 15, 55, 80},        {"Idle", 3, 90, 0, 2, 12},
    {"antivirus.exe", 60, 55, 40, 20, 35},
};

void TaskManager::initialize() {
  for (auto res : resources) {
    int sum = 0;
    for (auto &pd : inputVector)
      sum += pd.*res;
    if (sum > 100) {
      for (auto &pd : inputVector)
        pd.*res = pd.*res * 100 / sum;
    }
  }
  this->pds = inputVector;
}

void TaskManager::randomize() {
  static double lastTime = 0.0;
  double now = ImGui::GetTime();
  if (now - lastTime < 0.5)
    return;
  lastTime = now;

  static std::mt19937 rng(std::random_device{}());
  std::uniform_int_distribution<int> delta(-5, 5);

  for (auto res : resources) {
    int sum = 0;
    for (auto &pd : pds)
      sum += pd.*res;

    for (auto &pd : pds) {
      int d = delta(rng);
      if (d > 0) {
        int increase = std::min(d, std::min(100 - pd.*res, 100 - sum));
        pd.*res += increase;
        sum += increase;
      } else {
        int decrease = std::min(-d, pd.*res);
        pd.*res -= decrease;
        sum -= decrease;
      }
    }
  }
}

void TaskManager::display() {}
