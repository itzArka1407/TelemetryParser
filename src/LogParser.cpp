#include "LogParser.h"
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <string_view>
#include <thread>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

LogParser::LogParser(const char *file_path)
    : file_path(file_path), counts{0, 0, 0, 0, 0} {

  // Persist the file descriptor open across the life of the analyzer
  fd = open(file_path, O_RDONLY);
  if (fd == -1) {
    std::cerr << "Failed to open file: " << file_path << "\n";
    return;
  }

  LogParser::remap_if_grown(); // Remap the mapping of virtual mem addr to
                               // physical mem addr
}

LogParser::~LogParser() {
  if (mmap_ptr != nullptr && file_size > 0) {
    munmap(const_cast<char *>(mmap_ptr), file_size);
  }
  if (fd != -1) {
    close(fd);
  }
}

void LogParser::remap_if_grown() {
  struct stat file_info;
  if (fstat(fd, &file_info) == -1)
    return;

  size_t actual_disk_size = file_info.st_size;
  if (actual_disk_size <= file_size)
    return; // Sizes match, nothing to do

  if (mmap_ptr == nullptr) {
    // Initial mapping run
    void *mapped_region =
        mmap(nullptr, actual_disk_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mapped_region != MAP_FAILED) {
      mmap_ptr = static_cast<const char *>(mapped_region);
      file_size = actual_disk_size;
    }
  } else {
    void *remapped_region = mremap(const_cast<char *>(mmap_ptr), file_size,
                                   actual_disk_size, MREMAP_MAYMOVE);
    if (remapped_region != MAP_FAILED) {
      mmap_ptr = static_cast<const char *>(remapped_region);
      file_size = actual_disk_size;
    }
  }
}

__attribute__((always_inline)) inline LogParser::LogType
LogParser::resolve_log_type(std::string_view log_line) const {
  // Log format: [dd:MM:yyyy hh:mm:ss LOG_TYPE] {Log} --> at idx: 21, the
  // LOG_TYPE starts(with max len: 5), so read from it
  std::string_view log = log_line.substr(21, 5);
  if (log == "INFO-")
    return LogType::INFO;
  if (log == "WARN-")
    return LogType::WARN;
  if (log == "ERROR")
    return LogType::ERROR;
  if (log == "OTHER")
    return LogType::OTHER;
  return LogType::UNKNOWN;
}

void LogParser::run_live(std::atomic<bool> &shutdown_flag) {
  if (fd == -1 || mmap_ptr == nullptr)
    return;

  while (!shutdown_flag.load(std::memory_order_relaxed)) {
    // Check if file has expanded on the disk
    remap_if_grown();

    // Consume new unparsed text slices inside our updated virtual memory bounds
    while (current_pos < file_size) {
      size_t line_start_idx = current_pos;

      const char *remaining_start = mmap_ptr + current_pos;
      size_t remaining_bytes = file_size - current_pos;

      // Scan with vectorized memchr execution block
      const char *next_newline = static_cast<const char *>(
          std::memchr(remaining_start, '\n', remaining_bytes));

      size_t line_len = 0;
      if (next_newline != nullptr) {
        size_t newline_offset = next_newline - mmap_ptr;
        line_len = newline_offset - line_start_idx;
        current_pos = newline_offset + 1; // Slide past layout marker
      } else {
        // Current tail block chunk handling
        line_len = file_size - line_start_idx;
        current_pos = file_size;
      }

      if (line_len > 0 && mmap_ptr[line_start_idx + line_len - 1] == '\r') {
        --line_len;
      }

      std::string_view line(&mmap_ptr[line_start_idx], line_len);
      if (line_len < 30 || !line.starts_with("["))
        continue; // Invalid line for a log, not a logging line

      LogType log = resolve_log_type(line);
      counts[static_cast<size_t>(log)].fetch_add(1, std::memory_order_relaxed);
    }

    // Sleep before performing the next check
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}
