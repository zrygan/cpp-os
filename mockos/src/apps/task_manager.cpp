#include "task_manager.h"
#include "imgui.h"
#include "context.h"
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

void TaskManager::updateMetrics() {
  static double lastTime = 0.0;
  double now = ImGui::GetTime();
  if (now - lastTime < 1.0) return;
  lastTime = now;

  static std::mt19937 rng(std::random_device{}());
  std::uniform_real_distribution<float> delta(-0.1, 0.1);

  // weights are arranged for CPU, Memory, GPU, Disk, Network usage
  static std::vector<float> weights = { 4.0, 3.0, 2.0, 1.0, 1.0 };
  static float totalWeight = 11.0;

  float raw[5];
  float sum = 0;
  for (int i = 0; i < 5; i++) {
    raw[i] = (weights[i] / totalWeight) + delta(rng);
    raw[i] = std::max(0.01f, raw[i]);
    sum += raw[i];
  }

  std::vector<float*> ptrs = { &performance.cpu, &performance.memory, &performance.gpu,
                    &performance.disk, &performance.network };
                    
  for (int i = 0; i < 5; i++) *ptrs[i] = raw[i] / sum;
}