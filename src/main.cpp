#include <fstream>
#include <iostream>
#include <ostream>
#include <string>

int main() {
  std::cout << "C++ env loaded successfully." << std::endl;

  std::string file_path = "server.log";
  std::ifstream log_file(file_path);

  if (!log_file.is_open()) {
    std::cerr << "Couldn't open the file at " << file_path << std::endl;
    return 1;
  }

  std::string line; // mutable string buffer to load lines from file
  while (std::getline(log_file, line)) {
    std::cout << "Line read as: " << line
              << "\n"; // No std::endl -- to prevent repetitive flushes
  }

  std::cout << "Reached file end -- file closed automatically" << std::endl;

  return 0;
}
