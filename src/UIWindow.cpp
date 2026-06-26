#include "UIWindow.hpp"
#include "GLFW/glfw3.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <implot.h>
#include <iostream>
#include <optional>

std::optional<GLFWwindow *> generate_window() {
  if (!glfwInit()) {
    std::cerr << "Cannot init GLFW" << std::endl;
    return std::nullopt;
  }

  // Config openGL context versions for GLFW
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE); // Resizable window
  glfwWindowHint(GLFW_DECORATED, GLFW_TRUE); // The maximize, minimize buttons

  GLFWwindow *window =
      glfwCreateWindow(1280, 720, "Log Analyzer", nullptr, nullptr);
  if (!window) {
    std::cerr << "Failed to create window" << std::endl;
    return std::nullopt;
  }

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  return window;
}
