#pragma once

#include "LogParser.h"
class AppWindow {
public:
  explicit AppWindow();                 // Create the app window
  void Render(const LogParser &parser); // Render the app window o screen
};
