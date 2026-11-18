#include "flightlab/scenario_runner.hpp"

#include <cmath>
#include <stdexcept>

namespace flightlab {
ScenarioResult run_scenario(DynamicsBackend& backend, const ScenarioConfig& config,
                            const std::function<void(const AircraftState&)>& observe) {
  if (!std::isfinite(config.timestep_s) || !std::isfinite(config.duration_s) ||
      config.timestep_s <= 0.0 || config.duration_s < 0.0) {
    throw std::invalid_argument("scenario duration and timestep must be finite; timestep positive");
  }
  const auto whole_steps = static_cast<unsigned>(std::floor(config.duration_s / config.timestep_s + 1e-9));
  observe(backend.state());
  for (unsigned i = 0; i < whole_steps; ++i) {
    backend.step(config.timestep_s);
    const auto current = backend.state();
    if (!finite(current)) return {i + 1, false, "nonfinite-state"};
    observe(current);
  }
  return {whole_steps + 1, true, "duration-complete"};
}
} // namespace flightlab
