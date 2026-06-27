#pragma once
#include <array>
#include <atomic>
#include <cstddef>
#include <string_view>

class LogParser {
public:
  const static size_t LOG_TYPES_COUNT = 5;

  // The type of log
  enum class LogType {
    INFO = 0,
    WARN = 1,
    ERROR = 2,
    UNKNOWN = 3,
    OTHER = 4,
  };

private:
  std::array<std::atomic<size_t>, LOG_TYPES_COUNT> counts;
  const char *file_path;
  int fd = -1; // Track file descriptor persistently

  const char *mmap_ptr = nullptr;
  size_t file_size = 0;   // Current size of virtual mem window
  size_t current_pos = 0; // Tracks where scanning was left off

  __attribute__((always_inline)) LogType
  resolve_log_type(std::string_view log_line) const;
  void remap_if_grown(); // Core dynamic engine worker

public:
  explicit LogParser(const char *file_path);
  ~LogParser();

  // The live streaming execution pass loop
  void run_live(std::atomic<bool> &shutdown_flag);

  // Convert atomic into normal -- used to read from it
  void copy_counts(long long *dest_buf) const {
    for (size_t i = 0; i < LOG_TYPES_COUNT; ++i) {
      dest_buf[i] =
          static_cast<long long>(counts[i].load(std::memory_order_relaxed));
    }
  }
};
