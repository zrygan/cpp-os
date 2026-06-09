#pragma once
#include <chrono>
#include <ctime>
#include <string>

namespace mockos {
/**
 * This gets the system clock and formats it.
 * @param include_seconds Toggle to include seconds in the output format.
 */
inline std::string GetTimeString(bool include_seconds = false) {
  auto now = std::chrono::system_clock::now();
  std::time_t time_now = std::chrono::system_clock::to_time_t(now);
  std::tm *local_time = std::localtime(&time_now);
  char buf[16];

  // Choose the format string based on the toggle
  const char *format = include_seconds ? "%I:%M:%S %p" : "%I:%M %p";

  std::strftime(buf, sizeof(buf), format, local_time);
  return buf;
}
} // namespace mockos