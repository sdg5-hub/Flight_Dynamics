#include "flightlab/steady_flight_backend.hpp"

#include <cmath>
#include <stdexcept>

namespace flightlab {
void SteadyFlightBackend::reset(const AircraftState& initial_state) {
  if (!finite(initial_state) || initial_state.true_airspeed_mps < 0.0) {
    throw std::invalid_argument("initial state must be finite with nonnegative airspeed");
  }
  state_ = initial_state;
}

void SteadyFlightBackend::step(const double dt_s) {
  if (!std::isfinite(dt_s) || dt_s <= 0.0) throw std::invalid_argument("timestep must be finite and positive");
  state_.sim_time_s += dt_s;
  // Great-circle approximation over a very short interval, with no wind or dynamics.
  constexpr double earth_radius_m = 6371000.0;
  const double distance = state_.true_airspeed_mps * dt_s;
  state_.latitude_rad += distance * std::cos(state_.heading_rad) / earth_radius_m;
  const double latitude_cosine = std::cos(state_.latitude_rad);
  if (std::abs(latitude_cosine) > 1e-12) {
    state_.longitude_rad += distance * std::sin(state_.heading_rad) / (earth_radius_m * latitude_cosine);
  }
}
} // namespace flightlab
