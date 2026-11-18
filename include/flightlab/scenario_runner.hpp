#pragma once

#include <functional>
#include "flightlab/dynamics_backend.hpp"

namespace flightlab {
struct ScenarioConfig { double timestep_s{0.005}; double duration_s{10.0}; unsigned seed{1}; };
struct ScenarioResult { unsigned samples{}; bool completed{}; const char* termination_reason{}; };

ScenarioResult run_scenario(DynamicsBackend& backend, const ScenarioConfig& config,
                            const std::function<void(const AircraftState&)>& observe);
} // namespace flightlab
