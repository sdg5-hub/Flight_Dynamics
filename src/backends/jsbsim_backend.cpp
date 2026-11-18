#include "flightlab/jsbsim_backend.hpp"

#include <cmath>
#include <stdexcept>

#include <FGFDMExec.h>
#include <initialization/FGInitialCondition.h>
#include <initialization/FGTrim.h>
#include <simgear/misc/sg_path.hxx>

namespace flightlab {
namespace {
constexpr double kFeetToMetres = 0.3048;
constexpr double kFeetPerSecondToMps = 0.3048;
constexpr double kMetresToFeet = 1.0 / kFeetToMetres;
constexpr double kMpsToKnots = 1.9438444924406;
constexpr double kEarthRadiusM = 6371000.0;

SGPath path_from(const std::filesystem::path& path) {
  return SGPath(path.string());
}
}

JsbsimBackend::JsbsimBackend(std::filesystem::path data_root) : data_root_(std::move(data_root)) {
  if (!std::filesystem::exists(data_root_ / "aircraft" / "c172r" / "c172r.xml")) {
    throw std::runtime_error("JSBSim C172R model is missing beneath the selected data root");
  }
  executive_ = std::make_unique<JSBSim::FGFDMExec>();
  executive_->SetDebugLevel(0);
  executive_->SetRootDir(path_from(data_root_));
  executive_->SetAircraftPath(SGPath("aircraft"));
  executive_->SetEnginePath(SGPath("engine"));
  executive_->SetSystemsPath(SGPath("systems"));
  if (!executive_->LoadModel("c172r")) throw std::runtime_error("JSBSim could not load c172r");
  executive_->DisableOutput();
}

JsbsimBackend::~JsbsimBackend() = default;

void JsbsimBackend::reset(const AircraftState& initial_state) {
  if (!finite(initial_state) || initial_state.true_airspeed_mps <= 0.0) {
    throw std::invalid_argument("JSBSim initial state must be finite with positive true airspeed");
  }
  const auto initial_conditions = executive_->GetIC();
  initial_conditions->SetLatitudeRadIC(initial_state.latitude_rad);
  initial_conditions->SetLongitudeRadIC(initial_state.longitude_rad);
  initial_conditions->SetAltitudeASLFtIC(initial_state.altitude_m * kMetresToFeet);
  initial_conditions->SetVtrueKtsIC(initial_state.true_airspeed_mps * kMpsToKnots);
  initial_conditions->SetPsiRadIC(initial_state.heading_rad);
  origin_latitude_rad_ = initial_state.latitude_rad;
  origin_longitude_rad_ = initial_state.longitude_rad;
  configured_dt_s_ = 0.0;
  executive_->SetPropertyValue("fcs/aileron-cmd-norm", 0.0);
  executive_->SetPropertyValue("fcs/elevator-cmd-norm", 0.0);
  executive_->SetPropertyValue("fcs/rudder-cmd-norm", 0.0);
  executive_->SetPropertyValue("fcs/throttle-cmd-norm", 0.50);
  executive_->SetPropertyValue("fcs/mixture-cmd-norm", 1.0);
  executive_->SetPropertyValue("propulsion/magneto_cmd", 3.0);
  executive_->SetPropertyValue("propulsion/starter_cmd", 1.0);
  if (!executive_->RunIC()) throw std::runtime_error("JSBSim RunIC failed");
  JSBSim::FGTrim trim(executive_.get(), JSBSim::tFull);
  if (!trim.DoTrim()) throw std::runtime_error("JSBSim c172r trim failed for the requested initial condition");
  // FGTrim enables JSBSim model-configured outputs while iterating; FlightLab owns telemetry.
  executive_->DisableOutput();
  if (!executive_->RunIC()) throw std::runtime_error("JSBSim RunIC failed after trim");
  actuators_ = {};
  actuators_.throttle_normalized = 0.50;
}

void JsbsimBackend::set_control_command(const ControlCommand& command) {
  if (!std::isfinite(command.aileron_normalized) || !std::isfinite(command.elevator_normalized) ||
      !std::isfinite(command.throttle_normalized)) {
    throw std::invalid_argument("JSBSim control command must be finite");
  }
  actuators_.aileron_normalized = clamp(command.aileron_normalized, -1.0, 1.0);
  actuators_.elevator_normalized = clamp(command.elevator_normalized, -1.0, 1.0);
  actuators_.throttle_normalized = clamp(command.throttle_normalized, 0.0, 1.0);
  executive_->SetPropertyValue("fcs/aileron-cmd-norm", actuators_.aileron_normalized);
  executive_->SetPropertyValue("fcs/elevator-cmd-norm", actuators_.elevator_normalized);
  executive_->SetPropertyValue("fcs/throttle-cmd-norm", actuators_.throttle_normalized);
}

void JsbsimBackend::step(const double dt_s) {
  if (!std::isfinite(dt_s) || dt_s <= 0.0 || dt_s > 0.1) {
    throw std::invalid_argument("JSBSim timestep must be finite, positive, and no greater than 0.1 seconds");
  }
  if (configured_dt_s_ == 0.0) configured_dt_s_ = dt_s;
  if (std::abs(configured_dt_s_ - dt_s) > 1e-12) {
    throw std::invalid_argument("JSBSim backend requires a fixed timestep after initialization");
  }
  executive_->Setdt(dt_s);
  if (!executive_->Run()) throw std::runtime_error("JSBSim Run returned false");
}

AircraftState JsbsimBackend::state() const {
  AircraftState result{};
  result.sim_time_s = executive_->GetSimTime();
  result.latitude_rad = executive_->GetPropertyValue("position/lat-gc-rad");
  result.longitude_rad = executive_->GetPropertyValue("position/long-gc-rad");
  result.north_m = (result.latitude_rad - origin_latitude_rad_) * kEarthRadiusM;
  result.east_m = wrap_angle_rad(result.longitude_rad - origin_longitude_rad_) *
      kEarthRadiusM * std::cos(origin_latitude_rad_);
  result.altitude_m = executive_->GetPropertyValue("position/h-sl-ft") * kFeetToMetres;
  result.heading_rad = wrap_angle_rad(executive_->GetPropertyValue("attitude/psi-rad"));
  result.true_airspeed_mps = executive_->GetPropertyValue("velocities/vtrue-fps") * kFeetPerSecondToMps;
  result.groundspeed_mps = executive_->GetPropertyValue("velocities/vg-fps") * kFeetPerSecondToMps;
  result.bank_rad = executive_->GetPropertyValue("attitude/phi-rad");
  result.pitch_rad = executive_->GetPropertyValue("attitude/theta-rad");
  return result;
}
} // namespace flightlab
