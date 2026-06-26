#include "AppWindow.hpp"
#include "LogParser.h"
#include "UIWindow.hpp"
#include "imgui.h"
#include "implot.h"
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <atomic>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include <ostream>
#include <thread>

// Packet layout upgraded to provide the UI loop with thread-safe access to the
// parser data
struct AppContext {
  GLFWwindow *window;
  AppWindow *appUI;
  const LogParser *parser; // Shared reference link to our live data engine
};

// Unified rendering step accessible by both the main loop and OS events
void render_frame(AppContext *ctx) {
  // Start the frame of ImGui
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  // Pass the parser reference into the rendering engine layout
  ctx->appUI->Render(*(ctx->parser));
  ImGui::Render(); // Render the components in frame

  // Dynamically query current dimensions inside the render sequence
  int display_w, display_h;
  glfwGetFramebufferSize(ctx->window, &display_w, &display_h);
  glViewport(0, 0, display_w, display_h);

  glClear(GL_COLOR_BUFFER_BIT);
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  glfwSwapBuffers(ctx->window);
}

// Intercepts the window manager's redraw event while the cursor is actively
// resizing the frame
static void window_refresh_callback(GLFWwindow *window) {
  AppContext *ctx = static_cast<AppContext *>(glfwGetWindowUserPointer(window));
  if (ctx) {
    render_frame(ctx);
  }
}

static void framebuffer_size_callback(GLFWwindow *window, int width,
                                      int height) {
  glViewport(0, 0, width, height);
}

int main(int argc, char *args[]) {
  // Enforce command-line safety check
  if (argc < 2) {
    std::cout << "Usage: ./log_analyzer <path_to_log_file>\n";
    return 1;
  }

  // 1. Instantiate the Parser Engine on the main thread stack
  LogParser parser(args[1]);

  // 2. Initialize the shutdown flag and spawn the background worker thread
  std::atomic<bool> shutdown_flag{false};
  std::thread worker_thread(&LogParser::run_live, &parser,
                            std::ref(shutdown_flag));

  AppWindow appUI;

  // Initialize the ImGui Context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImPlot::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

  ImGui::StyleColorsDark();

  // Generate the UI window, then assign it for display
  auto window = generate_window();
  if (!window.has_value()) {
    std::cout << "This is wrong -- cannot open the UI window" << std::endl;

    // Safety fallback: shut down worker before returning
    shutdown_flag.store(true);
    if (worker_thread.joinable())
      worker_thread.join();
    return -1;
  }
  auto ui_window = *window;

  // Set up synchronization pipeline hooks with the complete context pack
  AppContext ctx = {ui_window, &appUI, &parser};
  glfwSetWindowUserPointer(ui_window, &ctx);

  glfwSetFramebufferSizeCallback(ui_window, framebuffer_size_callback);
  glfwSetWindowRefreshCallback(ui_window, window_refresh_callback);

  ImGui_ImplGlfw_InitForOpenGL(ui_window, true);
  ImGui_ImplOpenGL3_Init("#version 330");

  // Pure event driven/polling rendering loop
  while (!glfwWindowShouldClose(ui_window)) {
    glfwWaitEventsTimeout(1.0 / 60.0); // Block and wait for at least one event
    glfwPollEvents();
    render_frame(&ctx);
  }

  // Window closed! Trigger thread teardown sequence
  std::cout << "Window closure detected. Stopping background log sync...\n";
  shutdown_flag.store(true); // Signal the worker loop to break execution

  if (worker_thread.joinable()) {
    worker_thread.join(); // Sync threads back up before memory cleanup
    std::cout << "Background thread stopped safely.\n";
  }

  // Close and release the acquired resources
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();

  ImPlot::DestroyContext();
  ImGui::DestroyContext();

  glfwDestroyWindow(ui_window);
  glfwTerminate();
  return 0;
}
