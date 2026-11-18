#pragma once

#include <filesystem>
#include <functional>
#include <string>
#include <vector>
#include "flightlab/dynamics_backend.hpp"

#include "flightlab/autopilot.hpp"

namespace flightlab {
struct ScenarioConfig {
  std::string name{"heading-altitude-airspeed"};
  double timestep_s{0.01};
  double duration_s{90.0};
  unsigned seed{20261118};
  unsigned telemetry_interval_steps{5};
};

struct TelemetrySample {
  std::string backend;
  std::string scenario;
  std::string event;
  AircraftState state;
  AutopilotSetpoints setpoints;
  AutopilotStatus autopilot;
  ActuatorState actuators;
};

struct ScenarioResult {
  unsigned samples{};
  bool completed{};
  std::string termination_reason;
  std::vector<TelemetrySample> samples_for_metrics;
};

struct OpenLoopResult {
  unsigned samples{};
  bool completed{};
  std::string termination_reason;
  std::vector<TelemetrySample> samples_for_metrics;
};

ScenarioResult run_synthetic_autopilot_scenario(DynamicsBackend& backend, const ScenarioConfig& config,
                                                const std::function<void(const TelemetrySample&)>& observe);
OpenLoopResult run_open_loop_scenario(DynamicsBackend& backend, const ScenarioConfig& config,
                                      const std::function<void(const TelemetrySample&)>& observe);
} // namespace flightlab
