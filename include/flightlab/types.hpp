#pragma once

#include <cmath>
#include <stdexcept>

namespace flightlab {
constexpr double kPi = 3.14159265358979323846;

struct AircraftState {
  double sim_time_s{};
  double latitude_rad{};
  double longitude_rad{};
  double altitude_m{};
  double heading_rad{}; // Navigation heading: clockwise from north.
  double true_airspeed_mps{};
};

inline bool finite(const AircraftState& value) {
  return std::isfinite(value.sim_time_s) && std::isfinite(value.latitude_rad) &&
      std::isfinite(value.longitude_rad) && std::isfinite(value.altitude_m) &&
      std::isfinite(value.heading_rad) && std::isfinite(value.true_airspeed_mps);
}

struct BackendCapabilities { bool supports_trim{}; bool is_aircraft_dynamics{}; };
} // namespace flightlab
