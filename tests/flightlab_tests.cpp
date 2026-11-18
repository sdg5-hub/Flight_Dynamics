#include <cmath>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

#include "flightlab/pid.hpp"
#include "flightlab/scenario_runner.hpp"
#include "flightlab/trainer_dynamics_backend.hpp"
#include "flightlab/types.hpp"
#ifdef FLIGHTLAB_HAS_JSBSIM
#include "flightlab/jsbsim_backend.hpp"
#endif

namespace {
void expect(const bool condition, const std::string& message) {
  if (!condition) throw std::runtime_error(message);
}

flightlab::AircraftState initial_state() {
  return {.latitude_rad = 0.5, .longitude_rad = 0.0, .altitude_m = 900.0,
          .heading_rad = 0.0, .true_airspeed_mps = 55.0, .groundspeed_mps = 55.0};
}

void test_angles() {
  expect(std::abs(flightlab::wrap_angle_rad(3.0 * flightlab::kPi) - flightlab::kPi) < 1e-12,
         "angle wrapping must preserve pi");
  expect(std::abs(flightlab::wrap_angle_rad(-3.0 * flightlab::kPi / 2.0) - flightlab::kPi / 2.0) < 1e-12,
         "angle wrapping must handle negative turns");
}

void test_pid_saturation() {
  flightlab::DiscretePid pid({.proportional = 1.0, .integral = 1.0, .derivative = 0.0,
                              .output_min = -0.5, .output_max = 0.5, .anti_windup_gain = 4.0});
  for (int index = 0; index < 100; ++index) {
    expect(pid.update_from_error(100.0, 0.0, 0.01) <= 0.5, "PID must honor saturation");
  }
  expect(pid.saturated(), "PID must report saturation");
  const double recovered = pid.update_from_error(0.0, 0.0, 0.01);
  expect(std::abs(recovered) <= 0.5, "PID anti-windup recovery must remain bounded");
}

void test_synthetic_determinism() {
  flightlab::TrainerDynamicsBackend first;
  flightlab::TrainerDynamicsBackend second;
  first.reset(initial_state());
  second.reset(initial_state());
  const flightlab::ScenarioConfig config{.timestep_s = 0.01, .duration_s = 12.0, .seed = 42};
  const auto one = flightlab::run_synthetic_autopilot_scenario(first, config, [](const auto&) {});
  const auto two = flightlab::run_synthetic_autopilot_scenario(second, config, [](const auto&) {});
  expect(one.completed && two.completed, "synthetic scenarios must complete");
  expect(one.samples == 241 && one.samples == two.samples, "fixed-step telemetry sample count must be stable");
  const auto& left = one.samples_for_metrics.back().state;
  const auto& right = two.samples_for_metrics.back().state;
  expect(std::abs(left.north_m - right.north_m) < 1e-12 && std::abs(left.heading_rad - right.heading_rad) < 1e-12,
         "same seed and toolchain must replay identically");
  expect(left.heading_rad > 0.1, "heading command must produce a visible maneuver");
}

void test_invalid_timestep() {
  flightlab::TrainerDynamicsBackend backend;
  backend.reset(initial_state());
  bool rejected = false;
  try { backend.step(0.0); } catch (const std::invalid_argument&) { rejected = true; }
  expect(rejected, "backend must reject nonpositive timestep");
}

#ifdef FLIGHTLAB_HAS_JSBSIM
void test_jsbsim_trim_smoke() {
  flightlab::JsbsimBackend backend(FLIGHTLAB_DEFAULT_JSBSIM_ROOT);
  backend.reset({.latitude_rad = 0.5, .longitude_rad = 0.0, .altitude_m = 1200.0,
                 .heading_rad = 0.0, .true_airspeed_mps = 51.4444, .groundspeed_mps = 51.4444});
  backend.set_control_command({.throttle_normalized = 0.50});
  backend.step(0.01);
  expect(flightlab::finite(backend.state()), "JSBSim state must be finite after actual integration");
}
#endif
}

int main() {
  try {
    test_angles();
    test_pid_saturation();
    test_synthetic_determinism();
    test_invalid_timestep();
#ifdef FLIGHTLAB_HAS_JSBSIM
    test_jsbsim_trim_smoke();
#endif
  } catch (const std::exception& error) {
    std::cerr << "test failure: " << error.what() << '\n';
    return 1;
  }
  std::cout << "all FlightLab checks passed\n";
}
