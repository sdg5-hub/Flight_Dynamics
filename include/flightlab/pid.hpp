#pragma once

#include <cmath>
#include <stdexcept>

#include "flightlab/types.hpp"

namespace flightlab {
struct PidConfig {
  double proportional{};
  double integral{};
  double derivative{};
  double output_min{-1.0};
  double output_max{1.0};
  double anti_windup_gain{1.0};
};

class DiscretePid {
 public:
  explicit DiscretePid(PidConfig config) : config_(config) {
    if (!std::isfinite(config_.proportional) || !std::isfinite(config_.integral) ||
        !std::isfinite(config_.derivative) || config_.output_min >= config_.output_max) {
      throw std::invalid_argument("invalid PID configuration");
    }
  }

  void reset(double output = 0.0) {
    integral_state_ = clamp(output, config_.output_min, config_.output_max);
    previous_measurement_ = 0.0;
    has_previous_measurement_ = false;
    saturated_ = false;
  }

  [[nodiscard]] double update_from_error(double error, double measurement, double dt_s) {
    if (!std::isfinite(error) || !std::isfinite(measurement) || !std::isfinite(dt_s) || dt_s <= 0.0) {
      throw std::invalid_argument("PID input and timestep must be finite; timestep positive");
    }
    const double derivative_on_measurement = has_previous_measurement_
        ? -(measurement - previous_measurement_) / dt_s : 0.0;
    const double unsaturated = config_.proportional * error + integral_state_ +
        config_.derivative * derivative_on_measurement;
    const double output = clamp(unsaturated, config_.output_min, config_.output_max);
    // Back-calculation prevents an unreachable demand from accumulating indefinitely.
    integral_state_ += (config_.integral * error + config_.anti_windup_gain * (output - unsaturated)) * dt_s;
    previous_measurement_ = measurement;
    has_previous_measurement_ = true;
    saturated_ = output != unsaturated;
    return output;
  }

  [[nodiscard]] bool saturated() const { return saturated_; }
 private:
  PidConfig config_;
  double integral_state_{};
  double previous_measurement_{};
  bool has_previous_measurement_{};
  bool saturated_{};
};
} // namespace flightlab
