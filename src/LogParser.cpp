#include "LogParser.h"
#include <cstddef>
#include <cstring>
#include <iostream>
#include <string_view>

#include <fcntl.h>    // open() and O_RDONLY
#include <sys/mman.h> // mmap() and munmap()
#include <sys/stat.h> // fstat()
#include <unistd.h>   // For close()

LogParser::LogParser(const char *file_path)
    : file_path(file_path), counts{0, 0, 0, 0} {
  int fd = open(file_path, O_RDONLY);
  if (fd == -1) {
    std::cerr << "Failed to open file description for: " << file_path << "\n";
    return;
  }

  struct stat file_info;
  if (fstat(fd, &file_info) == -1) {
    std::cerr << "LINUX ERR: Failed to fetch file stats\n";
    close(fd);
    return;
  }
  file_size = file_info.st_size;

  if (file_size == 0) {
    std::cerr << "FILE ERR: Empty file, aborting operation\n";
    return;
  }

  // MAP file into virtual memory space
  // PROT_READ -> Read the file only(mem protection)
  // MAP_PRIVATE -> Writes won't be reflected onto disks(no use, file is only
  // being read)
  void *mapped_region = mmap(nullptr, file_size, PROT_READ, MAP_PRIVATE, fd, 0);

  if (mapped_region == MAP_FAILED) {
    std::cerr << "LINUX ERR: mmap allocation failed.\n";
    mmap_ptr = nullptr;
    file_size = 0;
  } else {
    mmap_ptr = static_cast<const char *>(mapped_region);
  }

  close(fd);
}

// For file size > 0 and the ptr being initialized, clean the associated
// elements
LogParser::~LogParser() {
  if (mmap_ptr != nullptr && file_size > 0) {
    munmap(const_cast<char *>(mmap_ptr), file_size);
  }
}

__attribute__((always_inline)) inline LogParser::LogType
LogParser::resolve_log_type(std::string_view log) const {
  if (log == "INFO-")
    return INFO;
  else if (log == "WARN-")
    return WARN;
  else if (log == "ERROR")
    return ERROR;
  else
    return UNKNOWN;
}

bool LogParser::run() {
  if (mmap_ptr == nullptr || file_size == 0) {
    std::cout << "Engine ERR: Not initialized\n";
    return false;
  }

  size_t current_pos = 0;

  while (current_pos < file_size) {
    size_t line_start_idx = current_pos;

    // --- SIMD Accelerated Search ---
    // Scan the remaining file slice using glibc's vectorized memchr engine
    const char *remaining_start = mmap_ptr + current_pos;
    size_t remaining_bytes = file_size - current_pos;

    const char *next_newline = static_cast<const char *>(
        std::memchr(remaining_start, '\n', remaining_bytes));

    if (next_newline != nullptr) {
      // Found a newline! Calculate exactly where it is relative to the start
      size_t newline_offset = next_newline - mmap_ptr;
      size_t line_len = newline_offset - line_start_idx;

      // Advance current_pos completely past the '\n' for the next iteration
      current_pos = newline_offset + 1;

      // Handle trailing carriage returns safely
      if (line_len > 0 && mmap_ptr[line_start_idx + line_len - 1] == '\r') {
        --line_len;
      }

      // Slice out our zero-allocation view
      std::string_view line(&mmap_ptr[line_start_idx], line_len);

      if (line_len < 30 || !line.starts_with("["))
        continue;

      std::string_view log_slice = line.substr(23, 5);
      LogType log = resolve_log_type(log_slice);
      counts[log]++;
    } else {
      // Edge case: The final line of the file doesn't end with a newline
      // character
      size_t line_len = file_size - line_start_idx;
      current_pos = file_size; // Break the outer loop

      std::string_view line(&mmap_ptr[line_start_idx], line_len);
      if (line_len < 30 || !line.starts_with("["))
        continue;

      std::string_view log_slice = line.substr(23, 5);
      LogType log = resolve_log_type(log_slice);
      counts[log]++;
    }
  }

  return true;
}
