#pragma once
#include <iostream>
#include <thread>

namespace mockos {
inline void TermResize(int rows, int cols) {
  std::cout << "\033[8;" << rows << ";" << cols << "t";
  std::cout.flush();
}

inline void TermClear() {
  std::cout << "\033[2J\033[H";
  std::cout.flush();
}

inline void TermMoveTo(int row, int col) {
  std::cout << "\033[" << row << ";" << col << "H";
}

inline void TermHideCursor() {
  std::cout << "\033[?25l";
  std::cout.flush();
}

inline void TermShowCursor() {
  std::cout << "\033[?25h";
  std::cout.flush();
}
} // namespace mockos
