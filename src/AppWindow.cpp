#include "AppWindow.hpp"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <implot.h>
AppWindow::AppWindow() {}

// Render the UI and set the layout for the screen
void AppWindow::Render() {
  // Main viewport config
  ImGuiViewport *viewport = ImGui::GetMainViewport();

  ImGui::SetNextWindowPos(viewport->WorkPos);
  ImGui::SetNextWindowSize(viewport->WorkSize);

  ImGuiWindowFlags flags =
      ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
      ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus;

  ImGui::Begin("Log Analyzer", nullptr, flags);
  ImGui::Separator();

  // TODO: Add actual labels' data from the file of logs
  static const char *labels[] = {"INFO", "WARN", "ERROR", "OTHER"};
  static double logs_count[] = {
      100, 300, 230, 119}; // FIXME: Remove this and search the log file

  if (ImPlot::BeginPlot("Log Entry Distributions", ImVec2(-1, -1))) {
    ImPlot::SetupAxes("Log Type", "Count");
    ImPlot::SetupAxisTicks(ImAxis_X1, 0, 3, 4, labels);
    ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 1500, ImGuiCond_Once);
    ImPlot::PlotBars("Occurences", &logs_count[0], 4, 0.4);
    ImPlot::EndPlot();
  }

  ImGui::End();
}
