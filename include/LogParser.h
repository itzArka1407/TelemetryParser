#pragma once
#include <array>
#include <cstddef>
#include <string_view>

class LogParser {
  std::array<size_t, 4> counts; // The counts of different log types
  const char *file_path;        // Path of the file

  enum LogType {
    INFO = 0,
    WARN = 1,
    ERROR = 2,
    UNKNOWN = 3,
  };

  __attribute__((always_inline)) LogType resolve_log_type(
      std::string_view log) const; // From a log string slice, get the log type

public:
  explicit LogParser(const char *file_path); // Constructor

  bool run();

  size_t get_info_count() const { return counts[0]; }
  size_t get_warn_count() const { return counts[1]; }
  size_t get_error_count() const { return counts[2]; }
  size_t get_unknown_count() const { return counts[3]; }
};
