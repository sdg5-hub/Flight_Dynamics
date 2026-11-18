#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "flightlab/scenario_runner.hpp"

namespace flightlab {
struct ExperimentMetrics {
  unsigned samples{};
  double duration_s{};
  double heading_rms_deg{};
  double altitude_rms_m{};
  double airspeed_rms_mps{};
  double max_bank_deg{};
  double actuator_saturation_fraction{};
  unsigned nonfinite_state_count{};
  bool completed{};
  std::string termination_reason;
};

ExperimentMetrics evaluate(const std::vector<TelemetrySample>& samples, bool completed,
                           std::string termination_reason);
void write_summary_json(const std::filesystem::path& path, const ExperimentMetrics& metrics,
                        const std::string& backend, const std::string& scenario,
                        const std::string& model_note);
} // namespace flightlab
