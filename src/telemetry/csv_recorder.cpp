#include "flightlab/csv_recorder.hpp"

#include <iomanip>
#include <stdexcept>

namespace flightlab {
CsvRecorder::CsvRecorder(const std::filesystem::path& path) : stream_(path) {
  if (!stream_) throw std::runtime_error("could not open telemetry output: " + path.string());
  stream_ << "schema_version,backend,scenario,event,mode,sim_time_s,latitude_rad,longitude_rad,north_m,east_m,altitude_m,heading_rad,true_airspeed_mps,groundspeed_mps,bank_rad,pitch_rad,heading_setpoint_rad,altitude_setpoint_m,airspeed_setpoint_mps,aileron_command,elevator_command,throttle_command,aileron_actual,elevator_actual,throttle_actual,heading_error_rad,altitude_error_m,airspeed_error_mps,heading_saturated,altitude_saturated,airspeed_saturated\n";
  stream_ << std::setprecision(17);
}
void CsvRecorder::record(const TelemetrySample& sample) {
  const auto& s = sample.state;
  const auto& a = sample.autopilot;
  const auto& u = sample.actuators;
  stream_ << "2," << sample.backend << ',' << sample.scenario << ',' << sample.event << ',' << a.mode << ','
          << s.sim_time_s << ',' << s.latitude_rad << ',' << s.longitude_rad << ',' << s.north_m << ','
          << s.east_m << ',' << s.altitude_m << ',' << s.heading_rad << ',' << s.true_airspeed_mps << ','
          << s.groundspeed_mps << ',' << s.bank_rad << ',' << s.pitch_rad << ',' << sample.setpoints.heading_rad
          << ',' << sample.setpoints.altitude_m << ',' << sample.setpoints.airspeed_mps << ','
          << a.command.aileron_normalized << ',' << a.command.elevator_normalized << ','
          << a.command.throttle_normalized << ',' << u.aileron_normalized << ',' << u.elevator_normalized << ','
          << u.throttle_normalized << ',' << a.heading_error_rad << ',' << a.altitude_error_m << ','
          << a.airspeed_error_mps << ',' << a.heading_saturated << ',' << a.altitude_saturated << ','
          << a.airspeed_saturated << '\n';
  if (!stream_) throw std::runtime_error("could not write telemetry");
}
} // namespace flightlab
