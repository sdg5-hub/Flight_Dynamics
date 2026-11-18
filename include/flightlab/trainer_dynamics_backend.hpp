#pragma once

#include "flightlab/dynamics_backend.hpp"

namespace flightlab {
// An educational reduced-order model, intentionally not an aircraft-fidelity claim.
class TrainerDynamicsBackend final : public DynamicsBackend {
 public:
  void reset(const AircraftState& initial_state) override;
  void set_control_command(const ControlCommand& command) override;
  void step(double dt_s) override;
  [[nodiscard]] AircraftState state() const override { return state_; }
  [[nodiscard]] ActuatorState actuators() const override { return actuators_; }
  [[nodiscard]] BackendCapabilities capabilities() const override { return {.supports_trim = false, .is_aircraft_dynamics = false, .is_synthetic = true}; }
  void set_crosswind_mps(double north_mps, double east_mps);
 private:
  static double rate_limit(double current, double requested, double limit_per_s, double dt_s);
  AircraftState state_{};
  ControlCommand command_{};
  ActuatorState actuators_{};
  double roll_rate_rps_{};
  double pitch_rate_rps_{};
  double wind_north_mps_{};
  double wind_east_mps_{};
};
} // namespace flightlab
