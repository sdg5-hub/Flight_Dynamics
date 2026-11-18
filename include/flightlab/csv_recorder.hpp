#pragma once

#include <filesystem>
#include <fstream>
#include "flightlab/scenario_runner.hpp"

namespace flightlab {
class CsvRecorder {
 public:
  explicit CsvRecorder(const std::filesystem::path& path);
  void record(const TelemetrySample& sample);
 private:
  std::ofstream stream_;
};
} // namespace flightlab
