#include "AppWindow.hpp"
#include "LogParser.h"
#include "UIWindow.hpp"
#include "imgui.h"
#include "implot.h"
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include <ostream>

// Packet layout to easily share pointers across GLFW callback boundaries
struct AppContext {
  GLFWwindow *window;
  AppWindow *appUI;
};

// Unified rendering step accessible by both the main loop and OS events
void render_frame(AppContext *ctx) {
  // Start the frame of ImGui
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  ctx->appUI->Render(); // Register the components onto the screen
  ImGui::Render();      // Render the components in frame

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
  // if (argc < 2) {
  //   std::cout << "No file path provided\n";
  //   return 1; // Err code
  // }
  //
  // LogParser parser(args[1]);
  //
  // if (!parser.run()) {
  //   return 1;
  // }
  //
  // std::cout << "\n=== LOG METRICS SUMMARY ===\n"
  //           << "  INFO  entries processed: " << parser.get_info_count() <<
  //           "\n"
  //           << "  WARN  entries processed: " << parser.get_warn_count() <<
  //           "\n"
  //           << "  ERROR entries processed: " << parser.get_error_count() <<
  //           "\n"
  //           << "  Malformed entry lines  : " << parser.get_unknown_count() <<
  //           "\n";
  //
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
    return -1;
  }
  auto ui_window = *window;

  // Set up synchronization pipeline hooks
  AppContext ctx = {ui_window, &appUI};
  glfwSetWindowUserPointer(ui_window, &ctx);

  glfwSetFramebufferSizeCallback(ui_window, framebuffer_size_callback);
  glfwSetWindowRefreshCallback(ui_window, window_refresh_callback);

  ImGui_ImplGlfw_InitForOpenGL(ui_window, true);
  ImGui_ImplOpenGL3_Init("#version 330");

  // Pure event driven/polling rendering loop
  while (!glfwWindowShouldClose(ui_window)) {
    glfwWaitEventsTimeout(1.0 / 60.0); // block and wait for atleast one event
    glfwPollEvents();
    render_frame(&ctx);
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
