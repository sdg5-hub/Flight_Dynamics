#pragma once

#include <filesystem>
#include <memory>

#include "flightlab/dynamics_backend.hpp"

namespace JSBSim { class FGFDMExec; }

namespace flightlab {
class JsbsimBackend final : public DynamicsBackend {
 public:
  explicit JsbsimBackend(std::filesystem::path data_root);
  ~JsbsimBackend() override;
  JsbsimBackend(const JsbsimBackend&) = delete;
  JsbsimBackend& operator=(const JsbsimBackend&) = delete;

  void reset(const AircraftState& initial_state) override;
  void set_control_command(const ControlCommand& command) override;
  void step(double dt_s) override;
  [[nodiscard]] AircraftState state() const override;
  [[nodiscard]] ActuatorState actuators() const override { return actuators_; }
  [[nodiscard]] BackendCapabilities capabilities() const override { return {.supports_trim = true, .is_aircraft_dynamics = true, .is_synthetic = false}; }
 private:
  std::filesystem::path data_root_;
  std::unique_ptr<JSBSim::FGFDMExec> executive_;
  ActuatorState actuators_{};
  double configured_dt_s_{};
  double origin_latitude_rad_{};
  double origin_longitude_rad_{};
};
} // namespace flightlab
