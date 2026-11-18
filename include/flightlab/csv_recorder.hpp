#pragma once

#include <filesystem>
#include <fstream>
#include "flightlab/types.hpp"

namespace flightlab {
class CsvRecorder {
 public:
  explicit CsvRecorder(const std::filesystem::path& path);
  void record(const AircraftState& state);
 private:
  std::ofstream stream_;
};
} // namespace flightlab
