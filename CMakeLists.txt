cmake_minimum_required(VERSION 4.2.0)
project(LogAnalyzer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

# Find System OpenGL
find_package(OpenGL REQUIRED)

add_compile_options(-O3 -march=native)

# ==============================================================================
# 1. Core Engine Target
# ==============================================================================
add_library(log_parser_core src/LogParser.cpp)
target_include_directories(log_parser_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# ==============================================================================
# 2. Third-Party Dependencies
# ==============================================================================

set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "")
set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "")
set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "")
set(GLFW_INSTALL OFF CACHE INTERNAL "")
set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.92.6
)
FetchContent_MakeAvailable(imgui)

add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC 
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)
target_link_libraries(imgui PUBLIC glfw OpenGL::GL)

FetchContent_Declare(
    implot
    GIT_REPOSITORY https://github.com/epezent/implot.git
    GIT_TAG        v1.0
)
FetchContent_MakeAvailable(implot)

add_library(implot STATIC
    ${implot_SOURCE_DIR}/implot.cpp
    ${implot_SOURCE_DIR}/implot_demo.cpp
    ${implot_SOURCE_DIR}/implot_items.cpp
)
target_include_directories(implot PUBLIC ${implot_SOURCE_DIR})
target_link_libraries(implot PUBLIC imgui)

# ==============================================================================
# 3. Main Application Target
# ==============================================================================
add_executable(log_analyzer 
    src/main.cpp 
    src/UIWindow.cpp
    src/AppWindow.cpp
)

# Added OpenGL::GL target here so the driver binaries resolve properly
target_link_libraries(log_analyzer PRIVATE log_parser_core implot imgui glfw OpenGL::GL)

# ==============================================================================
# 4. Testing
# ==============================================================================
enable_testing()
# add_executable(parser_benchmarks tests/benchmarks.cpp)
# target_link_libraries(parser_benchmarks PRIVATE log_parser_core)
# add_test(NAME ParserPerformance COMMAND parser_benchmarks)
