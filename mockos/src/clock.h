#pragma once
#include <chrono>
#include <ctime>
#include <string>

namespace mockos {
/**
 * This gets the system clock and formats it. This puts the time
 * inplace at the Context parameter.
 */
inline std::string GetTimeString() {
  auto now = std::chrono::system_clock::now();
  std::time_t time_now = std::chrono::system_clock::to_time_t(now);
  std::tm *local_time = std::localtime(&time_now);
  char buf[16];
  std::strftime(buf, sizeof(buf), "%I:%M %p", local_time);
  return buf;
}
} // namespace mockos
