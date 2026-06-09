
#include <chrono>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

// Strategy A: The standard scanning parser O(N)
void run_scanning_parser(const std::vector<std::string> &log_lines) {
  size_t error_count = 0;

  for (const auto &line : log_lines) {
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
  // Volatile escape to prevent the compiler from optimizing the loop away
  asm volatile("" : "+r"(error_count));
}

// Strategy B: Fixed-Width Parser O(1)
void run_fixed_width_parser(const std::vector<std::string> &log_lines) {
  size_t error_count = 0;

  for (const auto &line : log_lines) {
    std::string_view view(line);

    if (view.size() < 30 || view[0] != '[')
      continue; // Not a logging line

    std::string_view log_level = view.substr(23, 5);
    if (log_level == "ERROR") {
      error_count++;
    }
  }
  asm volatile("" : "+r"(error_count));
}

int main() {
  std::cout << "Generating 1,000,000 dummy log lines in memory...\n";
  std::vector<std::string> mock_logs;
  mock_logs.reserve(1000000);

  for (int i = 0; i < 1000000; ++i) {
    if (i % 3 == 0) {
      mock_logs.push_back(
          "[2026-06-08 23:15:12] [ERROR] Internal database connection dropped");
    } else if (i % 3 == 1) {
      mock_logs.push_back(
          "[2026-06-08 23:15:12] [WARN-] Connection retry limits exceeding");
    } else {
      mock_logs.push_back("[2026-06-08 23:15:12] [INFO-] Worker node reported "
                          "stable health status");
    }
  }

  std::cout << "Starting benchmark loops...\n\n";

  // Benchmark Scanning Parser
  auto start_scan = std::chrono::high_resolution_clock::now();
  run_scanning_parser(mock_logs);
  auto end_scan = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> scan_ms = end_scan - start_scan;

  // Benchmark Fixed-Width Parser
  auto start_fixed = std::chrono::high_resolution_clock::now();
  run_fixed_width_parser(mock_logs);
  auto end_fixed = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> fixed_ms = end_fixed - start_fixed;

  // Display Results
  std::cout << "=== PERFORMANCE RESULTS ===\n";
  std::cout << "O(N) Scanning Parser   : " << scan_ms.count() << " ms\n";
  std::cout << "O(1) Fixed-Width Parser: " << fixed_ms.count() << " ms\n";
  std::cout << "Speedup Factor         : "
            << (scan_ms.count() / fixed_ms.count()) << "x faster!\n";

  return 0;
}
