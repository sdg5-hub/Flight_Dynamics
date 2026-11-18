#include <cmath>
#include <exception>
#include <iostream>
#include "flightlab/scenario_runner.hpp"
#include "flightlab/steady_flight_backend.hpp"

int main() {
  try {
    flightlab::SteadyFlightBackend backend;
    backend.reset({0.0, 0.0, 0.0, 100.0, flightlab::kPi / 2.0, 10.0});
    unsigned observations = 0;
    const auto result = flightlab::run_scenario(backend, {0.1, 1.0, 42}, [&](const auto&) { ++observations; });
    const auto state = backend.state();
    if (!result.completed || result.samples != 11 || observations != 11 ||
        std::abs(state.sim_time_s - 1.0) > 1e-12 || state.longitude_rad <= 0.0) return 1;
    bool rejected = false;
    try { backend.step(0.0); } catch (const std::invalid_argument&) { rejected = true; }
    if (!rejected) return 1;
  } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
  std::cout << "all checks passed\n";
}
