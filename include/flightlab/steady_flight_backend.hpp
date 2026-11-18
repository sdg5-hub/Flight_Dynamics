#pragma once

#include "flightlab/dynamics_backend.hpp"

namespace flightlab {
// Deliberately minimal deterministic fallback for plumbing tests, not a flight model.
class SteadyFlightBackend final : public DynamicsBackend {
 public:
  void reset(const AircraftState& initial_state) override;
  void step(double dt_s) override;
  [[nodiscard]] AircraftState state() const override { return state_; }
  [[nodiscard]] BackendCapabilities capabilities() const override { return {}; }
 private:
  AircraftState state_{};
};
} // namespace flightlab
