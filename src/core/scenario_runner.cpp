#include "flightlab/scenario_runner.hpp"

#include <cmath>
#include <stdexcept>

#include "flightlab/trainer_dynamics_backend.hpp"

namespace flightlab {
namespace {
void validate(const ScenarioConfig& config) {
  if (!std::isfinite(config.timestep_s) || !std::isfinite(config.duration_s) ||
      config.timestep_s <= 0.0 || config.duration_s < 0.0 || config.telemetry_interval_steps == 0U) {
    throw std::invalid_argument("scenario duration/timestep must be finite, timestep and telemetry interval positive");
  }
}

std::string event_at(const double time_s) {
  if (time_s < 2.0) return "stabilize";
  if (time_s < 8.0) return "autopilot-engaged";
  if (time_s < 30.0) return "heading-step-30deg";
  if (time_s < 55.0) return "altitude-step-120m";
  if (time_s < 70.0) return "airspeed-step-5mps";
  if (time_s < 78.0) return "crosswind-12mps";
  return "recovery";
}
} // namespace

ScenarioResult run_synthetic_autopilot_scenario(DynamicsBackend& backend, const ScenarioConfig& config,
                                                const std::function<void(const TelemetrySample&)>& observe) {
  validate(config);
  const auto whole_steps = static_cast<unsigned>(std::floor(config.duration_s / config.timestep_s + 1e-9));
  CascadedAutopilot autopilot;
  autopilot.reset();
  AutopilotSetpoints setpoints{.heading_rad = 0.0, .altitude_m = 900.0, .airspeed_mps = 55.0};
  ScenarioResult result{};
  result.samples_for_metrics.reserve(static_cast<std::size_t>(whole_steps) + 1U);

  auto record = [&](const std::string& event, const AutopilotStatus& status) {
    TelemetrySample sample{.backend = std::string(kSyntheticModelLabel), .scenario = config.name,
                           .event = event, .state = backend.state(), .setpoints = setpoints,
                           .autopilot = status, .actuators = backend.actuators()};
    if (!finite(sample.state)) {
      result.completed = false;
      result.termination_reason = "nonfinite-state";
      return false;
    }
    observe(sample);
    result.samples_for_metrics.push_back(std::move(sample));
    return true;
  };
  AutopilotStatus initial{};
  initial.command.throttle_normalized = 0.48;
  if (!record("initial-condition", initial)) return result;

  for (unsigned i = 0; i < whole_steps; ++i) {
    const double time_s = backend.state().sim_time_s;
    const bool engaged = time_s >= 2.0;
    if (time_s >= 8.0) setpoints.heading_rad = kPi / 6.0;
    if (time_s >= 30.0) setpoints.altitude_m = 1020.0;
    if (time_s >= 55.0) setpoints.airspeed_mps = 60.0;
    if (auto* trainer = dynamic_cast<TrainerDynamicsBackend*>(&backend)) {
      trainer->set_crosswind_mps(time_s >= 70.0 && time_s < 78.0 ? 0.0 : 0.0,
                                 time_s >= 70.0 && time_s < 78.0 ? 12.0 : 0.0);
    }
    const AutopilotStatus status = autopilot.update(backend.state(), setpoints, config.timestep_s, engaged);
    backend.set_control_command(status.command);
    backend.step(config.timestep_s);
    if (((i + 1U) % config.telemetry_interval_steps == 0U || i + 1U == whole_steps) &&
        !record(event_at(backend.state().sim_time_s), status)) return result;
  }
  result.samples = static_cast<unsigned>(result.samples_for_metrics.size());
  result.completed = true;
  result.termination_reason = "duration-complete";
  return result;
}

OpenLoopResult run_open_loop_scenario(DynamicsBackend& backend, const ScenarioConfig& config,
                                      const std::function<void(const TelemetrySample&)>& observe) {
  validate(config);
  const auto whole_steps = static_cast<unsigned>(std::floor(config.duration_s / config.timestep_s + 1e-9));
  OpenLoopResult result{};
  result.samples_for_metrics.reserve(static_cast<std::size_t>(whole_steps) + 1U);
  const ControlCommand command{.aileron_normalized = 0.0, .elevator_normalized = 0.0, .throttle_normalized = 0.50};
  backend.set_control_command(command);
  for (unsigned i = 0; i <= whole_steps; ++i) {
    const auto state = backend.state();
    if (!finite(state)) {
      result.completed = false;
      result.termination_reason = "nonfinite-state";
      return result;
    }
    TelemetrySample sample{.backend = "jsbsim-c172r", .scenario = "open-loop-trim-reference",
                           .event = "open-loop-reference", .state = state,
                           .setpoints = {.heading_rad = state.heading_rad, .altitude_m = state.altitude_m,
                                         .airspeed_mps = state.true_airspeed_mps},
                           .autopilot = {.command = command, .mode = "OPEN LOOP"},
                           .actuators = backend.actuators()};
    if (i % config.telemetry_interval_steps == 0U || i == whole_steps) {
      observe(sample);
      result.samples_for_metrics.push_back(std::move(sample));
    }
    if (i < whole_steps) backend.step(config.timestep_s);
  }
  result.samples = static_cast<unsigned>(result.samples_for_metrics.size());
  result.completed = true;
  result.termination_reason = "duration-complete";
  return result;
}
} // namespace flightlab
