#include "LogParser.h"
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>

LogParser::LogParser(const char *file_path)
    : file_path(file_path), counts{0, 0, 0, 0} {}

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
  std::ifstream log_file(file_path); // Open the file

  if (!log_file.is_open()) {
    std::cout << "File opening failed at " << file_path << "\n";
    return false;
  }

  std::string line_buf; // Buffer to read the lines from the file
  while (std::getline(log_file, line_buf)) {
    std::string_view line(line_buf);

    // NOT a log line
    if (line.size() < 30 || !line.starts_with("["))
      continue;

    std::string_view log_slice = line.substr(23, 5); // Log's [ is at idx: 22
    LogType log_type = LogParser::resolve_log_type(log_slice);
    counts[log_type]++; // Increment the log type in the counts holder
  }

  return true;
}
