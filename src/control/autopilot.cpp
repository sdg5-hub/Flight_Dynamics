#include "flightlab/autopilot.hpp"

#include <stdexcept>

namespace flightlab {
CascadedAutopilot::CascadedAutopilot()
    : heading_to_bank_({.proportional = 1.25, .integral = 0.015, .derivative = 0.0,
                        .output_min = -0.44, .output_max = 0.44, .anti_windup_gain = 3.0}),
      bank_to_aileron_({.proportional = 3.2, .integral = 0.18, .derivative = 0.50,
                        .output_min = -1.0, .output_max = 1.0, .anti_windup_gain = 4.0}),
      altitude_to_pitch_({.proportional = 0.0080, .integral = 0.00015, .derivative = 0.0,
                          .output_min = -0.16, .output_max = 0.16, .anti_windup_gain = 1.5}),
      pitch_to_elevator_({.proportional = 4.2, .integral = 0.10, .derivative = 0.45,
                          .output_min = -1.0, .output_max = 1.0, .anti_windup_gain = 4.0}),
      airspeed_to_throttle_delta_({.proportional = 0.075, .integral = 0.012, .derivative = 0.0,
                                   .output_min = -0.26, .output_max = 0.35, .anti_windup_gain = 3.0}) {}

void CascadedAutopilot::reset() {
  heading_to_bank_.reset();
  bank_to_aileron_.reset();
  altitude_to_pitch_.reset();
  pitch_to_elevator_.reset();
  airspeed_to_throttle_delta_.reset();
}

AutopilotStatus CascadedAutopilot::update(const AircraftState& measured,
                                          const AutopilotSetpoints& setpoints,
                                          const double dt_s, const bool engaged) {
  if (!finite(measured) || !std::isfinite(setpoints.heading_rad) ||
      !std::isfinite(setpoints.altitude_m) || !std::isfinite(setpoints.airspeed_mps)) {
    throw std::invalid_argument("autopilot requires finite measurements and setpoints");
  }
  AutopilotStatus status{};
  if (!engaged) {
    status.command.throttle_normalized = 0.48;
    return status;
  }

  status.heading_error_rad = wrap_angle_rad(setpoints.heading_rad - measured.heading_rad);
  status.altitude_error_m = setpoints.altitude_m - measured.altitude_m;
  status.airspeed_error_mps = setpoints.airspeed_mps - measured.true_airspeed_mps;

  const double bank_command = heading_to_bank_.update_from_error(
      status.heading_error_rad, measured.heading_rad, dt_s);
  status.command.aileron_normalized = bank_to_aileron_.update_from_error(
      bank_command - measured.bank_rad, measured.bank_rad, dt_s);
  const double pitch_command = altitude_to_pitch_.update_from_error(
      status.altitude_error_m, measured.altitude_m, dt_s);
  status.command.elevator_normalized = pitch_to_elevator_.update_from_error(
      pitch_command - measured.pitch_rad, measured.pitch_rad, dt_s);
  status.command.throttle_normalized = clamp(0.48 + airspeed_to_throttle_delta_.update_from_error(
      status.airspeed_error_mps, measured.true_airspeed_mps, dt_s), 0.0, 1.0);

  status.heading_saturated = heading_to_bank_.saturated() || bank_to_aileron_.saturated();
  status.altitude_saturated = altitude_to_pitch_.saturated() || pitch_to_elevator_.saturated();
  status.airspeed_saturated = airspeed_to_throttle_delta_.saturated();
  status.mode = "HDG · ALT · IAS";
  return status;
}
} // namespace flightlab
