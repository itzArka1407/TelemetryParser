#include "LogParser.h"
#include <iostream>
#include <ostream>

int main(int argc, char *args[]) {
  if (argc < 2) {
    std::cout << "No file path provided\n";
    return 1; // Err code
  }

  LogParser parser(args[1]);

  if (!parser.run()) {
    return 1;
  }

  std::cout << "\n=== LOG METRICS SUMMARY ===\n"
            << "  INFO  entries processed: " << parser.get_info_count() << "\n"
            << "  WARN  entries processed: " << parser.get_warn_count() << "\n"
            << "  ERROR entries processed: " << parser.get_error_count() << "\n"
            << "  Malformed entry lines  : " << parser.get_unknown_count()
            << "\n";

  return 0;
}
