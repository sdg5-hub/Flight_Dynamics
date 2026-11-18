#pragma once

#include "flightlab/types.hpp"

namespace flightlab {
class DynamicsBackend {
 public:
  virtual ~DynamicsBackend() = default;
  virtual void reset(const AircraftState& initial_state) = 0;
  virtual void set_control_command(const ControlCommand& command) = 0;
  virtual void step(double dt_s) = 0;
  [[nodiscard]] virtual AircraftState state() const = 0;
  [[nodiscard]] virtual ActuatorState actuators() const = 0;
  [[nodiscard]] virtual BackendCapabilities capabilities() const = 0;
};
} // namespace flightlab
