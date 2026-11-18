#pragma once

#include <string>

#include "flightlab/pid.hpp"

namespace flightlab {
struct AutopilotSetpoints {
  double heading_rad{};
  double altitude_m{};
  double airspeed_mps{};
};

struct AutopilotStatus {
  ControlCommand command{};
  double heading_error_rad{};
  double altitude_error_m{};
  double airspeed_error_mps{};
  bool heading_saturated{};
  bool altitude_saturated{};
  bool airspeed_saturated{};
  std::string mode{"STANDBY"};
};

class CascadedAutopilot {
 public:
  CascadedAutopilot();
  void reset();
  [[nodiscard]] AutopilotStatus update(const AircraftState& measured, const AutopilotSetpoints& setpoints,
                                       double dt_s, bool engaged);
 private:
  DiscretePid heading_to_bank_;
  DiscretePid bank_to_aileron_;
  DiscretePid altitude_to_pitch_;
  DiscretePid pitch_to_elevator_;
  DiscretePid airspeed_to_throttle_delta_;
};
} // namespace flightlab
