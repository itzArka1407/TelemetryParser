#include "AppWindow.hpp"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <implot.h>
AppWindow::AppWindow() {}

// Render the UI and set the layout for the screen
void AppWindow::Render(const LogParser &parser) {
  ImGuiViewport *viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(viewport->WorkPos);
  ImGui::SetNextWindowSize(viewport->WorkSize);

  ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration |
                                  ImGuiWindowFlags_NoMove |
                                  ImGuiWindowFlags_NoSavedSettings;

  ImGui::Begin("Log Analyzer Dashboard", nullptr, window_flags);
  ImGui::Text("Live Log Stream Metrics");
  ImGui::Separator();

  // 1. Fetch the size_t metrics pointer
  long long local_chart_data[5] = {0};
  parser.copy_counts(local_chart_data);

  int max_val = 0;
  // Get the ceiling size of the chart to be displayed
  for (int i = 0; i < 5; ++i) {
    if (local_chart_data[i] > max_val)
      max_val = local_chart_data[i];
  }

  static const char *labels[] = {"INFO", "WARN", "ERROR", "UNKNOWN", "OTHER"};

  if (ImPlot::BeginPlot("Live Log Profile Distribution", ImVec2(-1, -1))) {
    ImPlot::SetupAxes("Log Categories", "Total Occurrences",
                      ImPlotAxisFlags_None, ImPlotAxisFlags_None);
    ImPlot::SetupAxisTicks(ImAxis_X1, 0, 4, 5, labels);

    // Adjust Y-limits using our standard int maximum snapshot
    double y_limit = static_cast<double>(max_val) * 1.15 + 10.0;
    ImPlot::SetupAxisLimits(ImAxis_Y1, 0, y_limit, ImGuiCond_Always);

    // 3. Pass the standard int array. The linker will match this instantly.
    ImPlot::PlotBars("Occurrences", local_chart_data, 5, 0.4);

    ImPlot::EndPlot();
  }

  ImGui::End();
}
