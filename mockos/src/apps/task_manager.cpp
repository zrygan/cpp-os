#include "task_manager.h"
#include "imgui.h"
#include <cstdio>
#include <random>
#include <algorithm>

void TaskManager::display() {
}

void TaskManager::randomize() {
    static double lastTime = 0.0;
    double now = ImGui::GetTime();
    if (now - lastTime < 0.5) return;
    lastTime = now;

    static std::mt19937 rng(std::random_device{}());
    static std::uniform_int_distribution<int> delta(-5, 5);

    for (auto &pd : pds) {
        auto clamp = [](int v) { return std::clamp(v, 0, 100); };
        pd.cpu     = clamp(pd.cpu     + delta(rng));
        pd.memory  = clamp(pd.memory  + delta(rng));
        pd.gpu     = clamp(pd.gpu     + delta(rng));
        pd.disk    = clamp(pd.disk    + delta(rng));
        pd.network = clamp(pd.network + delta(rng));
    }
}

void TaskManager::initialize(std::vector<ProcessDetail> inputVector) {
    this->pds = inputVector;
}
