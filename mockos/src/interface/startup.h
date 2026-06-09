#pragma once
#include "context.h"
#include "sys.h"
#include "term.h"
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <thread>
#include <vector>

namespace mockos {

const std::vector<std::string> kLogo = {
    "MMMMMMMM               MMMMMMMM                                     "
    "kkkkkkkk                OOOOOOOOO        SSSSSSSSSSSSSSS ",
    "M:::::::M             M:::::::M                                     "
    "k::::::k              OO:::::::::OO    SS:::::::::::::::S",
    "M::::::::M           M::::::::M                                     "
    "k::::::k            OO:::::::::::::OO S:::::SSSSSS::::::S",
    "M:::::::::M         M:::::::::M                                     "
    "k::::::k           O:::::::OOO:::::::OS:::::S     SSSSSSS",
    "M::::::::::M       M::::::::::M   ooooooooooo       cccccccccccccccc "
    "k:::::k    kkkkkkkO::::::O   O::::::OS:::::S            ",
    "M:::::::::::M     M:::::::::::M oo:::::::::::oo   cc:::::::::::::::c "
    "k:::::k   k:::::k O:::::O     O:::::OS:::::S            ",
    "M:::::::M::::M   M::::M:::::::Mo:::::::::::::::o c:::::::::::::::::c "
    "k:::::k  k:::::k  O:::::O     O:::::O S::::SSSS         ",
    "M::::::M M::::M M::::M M::::::Mo:::::ooooo:::::oc:::::::cccccc:::::c "
    "k:::::k k:::::k   O:::::O     O:::::O  SS::::::SSSSS    ",
    "M::::::M  M::::M::::M  M::::::Mo::::o     o::::oc::::::c     ccccccc "
    "k::::::k:::::k    O:::::O     O:::::O    SSS::::::::SS  ",
    "M::::::M   M:::::::M   M::::::Mo::::o     o::::oc:::::c              "
    "k:::::::::::k     O:::::O     O:::::O       SSSSSS::::S ",
    "M::::::M    M:::::M    M::::::Mo::::o     o::::oc:::::c              "
    "k:::::::::::k     O:::::O     O:::::O            S:::::S",
    "M::::::M     MMMMM     M::::::Mo::::o     o::::oc::::::c     ccccccc "
    "k::::::k:::::k    O::::::O   O::::::O            S:::::S",
    "M::::::M               "
    "M::::::Mo:::::ooooo:::::oc:::::::cccccc:::::ck::::::k k:::::k   "
    "O:::::::OOO:::::::OSSSSSSS     S:::::S",
    "M::::::M               M::::::Mo:::::::::::::::o "
    "c:::::::::::::::::ck::::::k  k:::::k   OO:::::::::::::OO "
    "S::::::SSSSSS:::::S",
    "M::::::M               M::::::M oo:::::::::::oo   "
    "cc:::::::::::::::ck::::::k   k:::::k    OO:::::::::OO   "
    "S:::::::::::::::SS ",
    "MMMMMMMM               MMMMMMMM   ooooooooooo       "
    "cccccccccccccccckkkkkkkk    kkkkkkk     OOOOOOOOO      SSSSSSSSSSSSSSS   ",
};

const int kFrameMs = 50;
const int kLogoPadCols = 8;
const int kLogoPadRows = 6;

inline int LogoWidth() {
  int w = 0;
  for (const auto &row : kLogo)
    if ((int)row.size() > w)
      w = (int)row.size();
  return w;
}

inline int LogoRows() { return (int)kLogo.size(); }
inline int GetRequiredCols() { return LogoWidth() + kLogoPadCols; }
inline int GetRequiredRows() { return LogoRows() + kLogoPadRows; }

inline void RunAnimation() {
  std::mt19937 rng(std::random_device{}());
  const std::string kNoisePool = "abcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*";
  std::uniform_int_distribution<int> noise_dist(0, (int)kNoisePool.size() - 1);

  int logo_width = LogoWidth();
  int logo_start_row = (GetRequiredRows() - LogoRows()) / 2;
  int logo_start_col = (GetRequiredCols() - logo_width) / 2 + 1;

  struct Pos {
    int r, c;
  };
  std::vector<Pos> positions;
  for (int r = 0; r < LogoRows(); ++r) {
    for (int c = 0; c < (int)kLogo[r].size(); ++c) {
      if (kLogo[r][c] != ' ') {
        positions.push_back({r, c});
      }
    }
  }

  std::shuffle(positions.begin(), positions.end(), rng);

  for (const auto &p : positions) {
    mockos::TermMoveTo(logo_start_row + p.r, logo_start_col + p.c);
    std::cout << "\033[32m" << kNoisePool[noise_dist(rng)] << "\033[0m";
  }
  std::cout.flush();
  std::this_thread::sleep_for(std::chrono::milliseconds(300));

  const int kRevealSpeed = 20;
  const int kFrameDelayMs = 15;
  size_t idx = 0;

  while (idx < positions.size()) {
    for (int i = 0; i < kRevealSpeed && idx < positions.size(); ++i, ++idx) {
      Pos p = positions[idx];
      mockos::TermMoveTo(logo_start_row + p.r, logo_start_col + p.c);
      std::cout << "\033[97m" << kLogo[p.r][p.c] << "\033[0m";
    }

    int flicker_count = std::min(15, (int)(positions.size() - idx));
    for (int i = 0; i < flicker_count; ++i) {
      std::uniform_int_distribution<size_t> random_unrevealed(
          idx, positions.size() - 1);
      Pos p = positions[random_unrevealed(rng)];
      mockos::TermMoveTo(logo_start_row + p.r, logo_start_col + p.c);
      std::cout << "\033[92m" << kNoisePool[noise_dist(rng)] << "\033[0m";
    }

    std::cout.flush();
    std::this_thread::sleep_for(std::chrono::milliseconds(kFrameDelayMs));
  }
}
inline void PrintPOST(const std::string &bios_ver, const std::string &bios_date,
                      const std::string &system_clock) {
  std::string cpu = mockos::GetCPUModel();
  long ram = mockos::GetTotalRAMMB();

  TermClear();
  std::cout << std::endl;
  std::cout << "  MockOS v1.0 \n";
  std::cout << "  By: Stephen Borja, Erin Chua, Zhean Ganituen, & Aaron Go"
            << std::endl;
  std::cout << "  For: CSOPESY [S06] Semi-MO - Desktop-Style OS Mockup"
            << std::endl;
  std::cout << "  ------------------------------------------------"
            << std::endl;
  std::cout << "  Processor    : " << cpu << std::endl;
  std::cout << "  Memory       : " << ram << " MB\n";
  std::cout << "  BIOS Version : " << bios_ver << std::endl;
  std::cout << "  BIOS Date    : " << bios_date << std::endl << std::endl;
  std::cout << "  System Clock : " << system_clock << std::endl;
  std::cout << "  ------------------------------------------------" << std::endl
            << std::endl;
  std::cout.flush();
}

inline void RunStartUp(const std::string &bios_ver,
                       const std::string &bios_date,
                       const std::string &system_clock) {
  TermResize(GetRequiredRows(), GetRequiredCols());
  TermHideCursor();
  TermClear();
  PrintPOST(bios_ver, bios_date, system_clock);
  std::this_thread::sleep_for(std::chrono::seconds(5));
  TermClear();
  RunAnimation();
  std::this_thread::sleep_for(std::chrono::seconds(1));
  TermShowCursor();
}

} // namespace mockos