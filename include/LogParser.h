#pragma once
#include <array>
#include <atomic>
#include <cstddef>
#include <string_view>

class LogParser {
public:
  // Shifted enum to public scope so ImPlot labels can cleanly match indices
  enum LogType {
    INFO = 0,
    WARN = 1,
    ERROR = 2,
    UNKNOWN = 3,
    OTHER = 4,
    COUNT = 5
  };

private:
  std::array<size_t, LogType::COUNT> counts; // Size updated to 5 elements
  const char *file_path;
  int fd = -1; // Track file descriptor persistently

  const char *mmap_ptr = nullptr;
  size_t file_size = 0;   // Current size of our virtual memory window
  size_t current_pos = 0; // Tracks exactly where we left off scanning

  __attribute__((always_inline)) LogType
  resolve_log_type(std::string_view log) const;
  void remap_if_grown(); // Core dynamic engine worker

public:
  explicit LogParser(const char *file_path);
  ~LogParser();

  // The live streaming execution pass loop
  void run_live(std::atomic<bool> &shutdown_flag);

  // Expose raw array pointer so ImPlot templates can directly read the data
  // slice
  const size_t *get_counts_ptr() const { return counts.data(); }

  size_t get_info_count() const { return counts[0]; }
  size_t get_warn_count() const { return counts[1]; }
  size_t get_error_count() const { return counts[2]; }
  size_t get_unknown_count() const { return counts[3]; }
  size_t get_other_count() const { return counts[4]; }
};
