#include "LogParser.h"
#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>

// Linux specific headers for the RAM file trick
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

// Fair Strategy A: Standard O(N) Scanning Parser + File I/O Overhead
size_t run_fair_scanning_parser(const char *path) {
  std::ifstream file(path);
  if (!file.is_open())
    return 0;

  size_t error_count = 0;
  std::string line;
  while (std::getline(file, line)) {
    std::string_view view(line);
    size_t time_start = view.find('[');
    size_t time_end = view.find(']');
    if (time_start == std::string_view::npos ||
        time_end == std::string_view::npos)
      continue;

    view.remove_prefix(time_end + 1);
    size_t lvl_start = view.find('[');
    size_t lvl_end = view.find(']');
    if (lvl_start == std::string_view::npos ||
        lvl_end == std::string_view::npos)
      continue;

    std::string_view log_level =
        view.substr(lvl_start + 1, lvl_end - lvl_start - 1);
    if (log_level == "ERROR") {
      error_count++;
    }
  }
  return error_count;
}

// Fair Strategy B: Fixed-Width Parser + File I/O Overhead
size_t run_fair_fixed_width_parser(const char *path) {
  std::ifstream file(path);
  if (!file.is_open())
    return 0;

  size_t error_count = 0;
  std::string line;
  while (std::getline(file, line)) {
    std::string_view view(line);
    if (view.size() < 30 || view[0] != '[')
      continue;

    std::string_view log_level = view.substr(23, 5);
    if (log_level == "ERROR") {
      error_count++;
    }
  }
  return error_count;
}

int main() {
  std::cout << "Generating 1,000,000 dummy log lines in memory...\n";
  std::string flat_serialized_buffer;
  flat_serialized_buffer.reserve(1000000 * 70);

  for (int i = 0; i < 1000000; ++i) {
    if (i % 3 == 0) {
      flat_serialized_buffer.append("[2026-06-08 23:15:12] [ERROR] Internal "
                                    "database connection dropped\n");
    } else if (i % 3 == 1) {
      flat_serialized_buffer.append(
          "[2026-06-08 23:15:12] [WARN-] Connection retry limits exceeding\n");
    } else {
      flat_serialized_buffer.append("[2026-06-08 23:15:12] [INFO-] Worker node "
                                    "reported stable health status\n");
    }
  }

  // Create virtual RAM file descriptor
  int mem_fd = memfd_create("mock_server_log", MFD_CLOEXEC);
  if (mem_fd == -1) {
    std::cerr << "Linux system fallback error: Failed to allocate RAM file.\n";
    return 1;
  }
  write(mem_fd, flat_serialized_buffer.data(), flat_serialized_buffer.size());
  std::string virtual_path = "/proc/self/fd/" + std::to_string(mem_fd);

  std::cout << "Starting fair benchmark loops (including File I/O opening "
               "initialization)...\n\n";

  // 1. Benchmark Fair Scanning Parser
  auto start_scan = std::chrono::high_resolution_clock::now();
  size_t scan_errors = run_fair_scanning_parser(virtual_path.c_str());
  auto end_scan = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> scan_ms = end_scan - start_scan;
  asm volatile("" ::"r"(scan_errors));

  // 2. Benchmark Fair Fixed-Width Parser
  auto start_fixed = std::chrono::high_resolution_clock::now();
  size_t fixed_errors = run_fair_fixed_width_parser(virtual_path.c_str());
  auto end_fixed = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> fixed_ms = end_fixed - start_fixed;
  asm volatile("" ::"r"(fixed_errors));

  // 3. Benchmark Vector-Accelerated mmap Engine Layout
  auto start_mmap = std::chrono::high_resolution_clock::now();
  size_t mmap_errors = 0;
  {
    LogParser mmap_engine(virtual_path.c_str());
    mmap_engine.run();
    mmap_errors = mmap_engine.get_error_count();
  }
  auto end_mmap = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> mmap_ms = end_mmap - start_mmap;
  asm volatile("" ::"r"(mmap_errors));

  close(mem_fd);

  // Display Results
  std::cout << "=== PERFORMANCE RESULTS ===\n";
  std::cout << "O(N) Stream Stream-Scanner: " << scan_ms.count() << " ms\n";
  std::cout << "O(1) Stream Fixed-Width   : " << fixed_ms.count() << " ms\n";
  std::cout << "SIMD Accelerated mmap     : " << mmap_ms.count() << " ms\n\n";

  std::cout << "🚀 Real Speedup (mmap vs Stream Scanning): "
            << (scan_ms.count() / mmap_ms.count()) << "x faster!\n";
  std::cout << "🚀 Real Speedup (mmap vs Stream Fixed)   : "
            << (fixed_ms.count() / mmap_ms.count()) << "x faster!\n";

  return 0;
}
