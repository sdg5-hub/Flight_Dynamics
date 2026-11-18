#pragma once

#include <algorithm>
#include <cmath>
#include <string_view>
#include <stdexcept>

namespace flightlab {
constexpr double kPi = 3.14159265358979323846;

struct AircraftState {
  double sim_time_s{};
  double latitude_rad{};
  double longitude_rad{};
  double north_m{};
  double east_m{};
  double altitude_m{};
  double heading_rad{}; // Navigation heading: clockwise from north.
  double true_airspeed_mps{};
  double groundspeed_mps{};
  double bank_rad{};
  double pitch_rad{};
};

inline bool finite(const AircraftState& value) {
  return std::isfinite(value.sim_time_s) && std::isfinite(value.latitude_rad) &&
      std::isfinite(value.longitude_rad) && std::isfinite(value.north_m) &&
      std::isfinite(value.east_m) && std::isfinite(value.altitude_m) &&
      std::isfinite(value.heading_rad) && std::isfinite(value.true_airspeed_mps) &&
      std::isfinite(value.groundspeed_mps) && std::isfinite(value.bank_rad) &&
      std::isfinite(value.pitch_rad);
}

struct ControlCommand {
  double aileron_normalized{};
  double elevator_normalized{};
  double throttle_normalized{0.48};
};

struct ActuatorState {
  double aileron_normalized{};
  double elevator_normalized{};
  double throttle_normalized{0.48};
};

inline double clamp(const double value, const double lower, const double upper) {
  return std::clamp(value, lower, upper);
}

inline double wrap_angle_rad(double value) {
  while (value > kPi) value -= 2.0 * kPi;
  while (value <= -kPi) value += 2.0 * kPi;
  return value;
}

inline constexpr std::string_view kSyntheticModelLabel = "synthetic-reduced-order-trainer";

struct BackendCapabilities { bool supports_trim{}; bool is_aircraft_dynamics{}; bool is_synthetic{}; };
} // namespace flightlab
