cmake_minimum_required(VERSION 3.20)
project(LogAnalyzer LANGUAGES CXX)

# Enforce Modern C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# CRITICAL: Forces CMake to generate the autocomplete database for Neovim
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Point to your source files
add_executable(log_analyzer src/main.cpp)

# Point to your headers folder
target_include_directories(log_analyzer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
