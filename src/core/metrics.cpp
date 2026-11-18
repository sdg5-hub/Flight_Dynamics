#include "flightlab/metrics.hpp"

#include <cmath>
#include <fstream>
#include <iomanip>
#include <stdexcept>

namespace flightlab {
namespace {
double rms(const std::vector<double>& values) {
  if (values.empty()) return 0.0;
  double sum{};
  for (const double value : values) sum += value * value;
  return std::sqrt(sum / static_cast<double>(values.size()));
}
}

ExperimentMetrics evaluate(const std::vector<TelemetrySample>& samples, const bool completed,
                           std::string termination_reason) {
  ExperimentMetrics metrics{};
  metrics.samples = static_cast<unsigned>(samples.size());
  metrics.completed = completed;
  metrics.termination_reason = std::move(termination_reason);
  if (samples.empty()) return metrics;
  metrics.duration_s = samples.back().state.sim_time_s - samples.front().state.sim_time_s;
  std::vector<double> heading_errors;
  std::vector<double> altitude_errors;
  std::vector<double> airspeed_errors;
  unsigned saturation_count{};
  for (const auto& sample : samples) {
    const auto& state = sample.state;
    if (!finite(state)) ++metrics.nonfinite_state_count;
    metrics.max_bank_deg = std::max(metrics.max_bank_deg, std::abs(state.bank_rad) * 180.0 / kPi);
    if (state.sim_time_s >= 16.0) heading_errors.push_back(sample.autopilot.heading_error_rad * 180.0 / kPi);
    if (state.sim_time_s >= 38.0) altitude_errors.push_back(sample.autopilot.altitude_error_m);
    if (state.sim_time_s >= 63.0) airspeed_errors.push_back(sample.autopilot.airspeed_error_mps);
    if (sample.autopilot.heading_saturated || sample.autopilot.altitude_saturated ||
        sample.autopilot.airspeed_saturated) ++saturation_count;
  }
  metrics.heading_rms_deg = rms(heading_errors);
  metrics.altitude_rms_m = rms(altitude_errors);
  metrics.airspeed_rms_mps = rms(airspeed_errors);
  metrics.actuator_saturation_fraction = static_cast<double>(saturation_count) / static_cast<double>(samples.size());
  return metrics;
}

void write_summary_json(const std::filesystem::path& path, const ExperimentMetrics& metrics,
                        const std::string& backend, const std::string& scenario,
                        const std::string& model_note) {
  std::ofstream stream(path);
  if (!stream) throw std::runtime_error("could not open summary output: " + path.string());
  stream << std::setprecision(12) << "{\n"
         << "  \"schema_version\": 1,\n"
         << "  \"backend\": \"" << backend << "\",\n"
         << "  \"scenario\": \"" << scenario << "\",\n"
         << "  \"model_note\": \"" << model_note << "\",\n"
         << "  \"completed\": " << (metrics.completed ? "true" : "false") << ",\n"
         << "  \"termination_reason\": \"" << metrics.termination_reason << "\",\n"
         << "  \"samples\": " << metrics.samples << ",\n"
         << "  \"duration_s\": " << metrics.duration_s << ",\n"
         << "  \"heading_rms_deg\": " << metrics.heading_rms_deg << ",\n"
         << "  \"altitude_rms_m\": " << metrics.altitude_rms_m << ",\n"
         << "  \"airspeed_rms_mps\": " << metrics.airspeed_rms_mps << ",\n"
         << "  \"max_bank_deg\": " << metrics.max_bank_deg << ",\n"
         << "  \"actuator_saturation_fraction\": " << metrics.actuator_saturation_fraction << ",\n"
         << "  \"nonfinite_state_count\": " << metrics.nonfinite_state_count << "\n}\n";
  if (!stream) throw std::runtime_error("could not write summary output");
}
} // namespace flightlab
