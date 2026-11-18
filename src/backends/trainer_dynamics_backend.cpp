#include "flightlab/trainer_dynamics_backend.hpp"

#include <cmath>
#include <stdexcept>

namespace flightlab {
namespace {
constexpr double kEarthRadiusM = 6371000.0;
constexpr double kGravityMps2 = 9.80665;
}

double TrainerDynamicsBackend::rate_limit(const double current, const double requested,
                                          const double limit_per_s, const double dt_s) {
  return current + clamp(requested - current, -limit_per_s * dt_s, limit_per_s * dt_s);
}

void TrainerDynamicsBackend::reset(const AircraftState& initial_state) {
  if (!finite(initial_state) || initial_state.true_airspeed_mps < 25.0 ||
      initial_state.altitude_m < 0.0) {
    throw std::invalid_argument("trainer initial state must be finite, airborne, and at least 25 m/s");
  }
  state_ = initial_state;
  state_.groundspeed_mps = state_.true_airspeed_mps;
  command_ = {};
  command_.throttle_normalized = 0.48;
  actuators_ = {};
  actuators_.throttle_normalized = 0.48;
  roll_rate_rps_ = 0.0;
  pitch_rate_rps_ = 0.0;
  wind_north_mps_ = 0.0;
  wind_east_mps_ = 0.0;
}

void TrainerDynamicsBackend::set_control_command(const ControlCommand& command) {
  if (!std::isfinite(command.aileron_normalized) || !std::isfinite(command.elevator_normalized) ||
      !std::isfinite(command.throttle_normalized)) {
    throw std::invalid_argument("control command must be finite");
  }
  command_.aileron_normalized = clamp(command.aileron_normalized, -1.0, 1.0);
  command_.elevator_normalized = clamp(command.elevator_normalized, -1.0, 1.0);
  command_.throttle_normalized = clamp(command.throttle_normalized, 0.0, 1.0);
}

void TrainerDynamicsBackend::set_crosswind_mps(const double north_mps, const double east_mps) {
  if (!std::isfinite(north_mps) || !std::isfinite(east_mps)) {
    throw std::invalid_argument("wind components must be finite");
  }
  wind_north_mps_ = north_mps;
  wind_east_mps_ = east_mps;
}

void TrainerDynamicsBackend::step(const double dt_s) {
  if (!std::isfinite(dt_s) || dt_s <= 0.0 || dt_s > 0.1) {
    throw std::invalid_argument("trainer timestep must be finite, positive, and no greater than 0.1 seconds");
  }
  actuators_.aileron_normalized = rate_limit(actuators_.aileron_normalized, command_.aileron_normalized, 2.6, dt_s);
  actuators_.elevator_normalized = rate_limit(actuators_.elevator_normalized, command_.elevator_normalized, 2.4, dt_s);
  actuators_.throttle_normalized = rate_limit(actuators_.throttle_normalized, command_.throttle_normalized, 0.6, dt_s);

  // Explicitly synthetic first-order attitude and energy response. Coefficients are not C172 data.
  const double roll_accel = 3.5 * actuators_.aileron_normalized - 2.1 * roll_rate_rps_ - 1.5 * state_.bank_rad;
  const double pitch_accel = 2.8 * actuators_.elevator_normalized - 1.8 * pitch_rate_rps_ - 1.15 * state_.pitch_rad;
  roll_rate_rps_ += roll_accel * dt_s;
  pitch_rate_rps_ += pitch_accel * dt_s;
  state_.bank_rad = clamp(state_.bank_rad + roll_rate_rps_ * dt_s, -0.78, 0.78);
  state_.pitch_rad = clamp(state_.pitch_rad + pitch_rate_rps_ * dt_s, -0.35, 0.35);

  const double airspeed_accel = 4.3 * (actuators_.throttle_normalized - 0.48) -
      0.070 * (state_.true_airspeed_mps - 55.0) - kGravityMps2 * std::sin(state_.pitch_rad);
  state_.true_airspeed_mps = clamp(state_.true_airspeed_mps + airspeed_accel * dt_s, 25.0, 95.0);
  const double heading_rate = kGravityMps2 * std::tan(state_.bank_rad) /
      std::max(state_.true_airspeed_mps, 25.0);
  state_.heading_rad = wrap_angle_rad(state_.heading_rad + heading_rate * dt_s);
  const double vertical_speed = state_.true_airspeed_mps * std::sin(state_.pitch_rad);
  state_.altitude_m = std::max(0.0, state_.altitude_m + vertical_speed * dt_s);

  const double north_speed = state_.true_airspeed_mps * std::cos(state_.heading_rad) + wind_north_mps_;
  const double east_speed = state_.true_airspeed_mps * std::sin(state_.heading_rad) + wind_east_mps_;
  state_.groundspeed_mps = std::hypot(north_speed, east_speed);
  state_.north_m += north_speed * dt_s;
  state_.east_m += east_speed * dt_s;
  state_.latitude_rad = 0.5 + state_.north_m / kEarthRadiusM;
  state_.longitude_rad = state_.east_m / (kEarthRadiusM * std::cos(0.5));
  state_.sim_time_s += dt_s;
}
} // namespace flightlab
