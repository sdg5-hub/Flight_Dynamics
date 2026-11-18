#include <filesystem>
#include <iostream>
#include "flightlab/csv_recorder.hpp"
#include "flightlab/scenario_runner.hpp"
#include "flightlab/steady_flight_backend.hpp"

int main(int argc, char** argv) {
  const std::filesystem::path output = argc > 1 ? argv[1] : "telemetry.csv";
  flightlab::SteadyFlightBackend backend;
  backend.reset({0.0, 0.0, 0.0, 500.0, flightlab::kPi / 2.0, 55.0});
  flightlab::CsvRecorder recorder(output);
  const auto result = flightlab::run_scenario(backend, {}, [&recorder](const auto& state) { recorder.record(state); });
  std::cout << "backend=steady-flight-fallback samples=" << result.samples
            << " completed=" << result.completed << " reason=" << result.termination_reason
            << " output=" << output << '\n';
  return result.completed ? 0 : 1;
}
