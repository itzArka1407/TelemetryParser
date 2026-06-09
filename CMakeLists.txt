cmake_minimum_required(VERSION 3.20)
project(LogAnalyzer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Global compilation optimization flags
add_compile_options(-O3 -march=native)

# 1. Define your core engine as a reusable Static Library
add_library(log_parser_core src/LogParser.cpp)

# Attach the include directory directly to the library target
target_include_directories(log_parser_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# 2. Main Application Target
add_executable(log_analyzer src/main.cpp)
# Link the core library (Brings in LogParser code and include directories automatically!)
target_link_libraries(log_analyzer PRIVATE log_parser_core)

# 3. Testing Engine (ctest) & Benchmark Target
enable_testing()

add_executable(parser_benchmarks tests/benchmarks.cpp)
target_link_libraries(parser_benchmarks PRIVATE log_parser_core)

add_test(NAME ParserPerformance COMMAND parser_benchmarks)
