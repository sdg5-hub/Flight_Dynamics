#include "flightlab/csv_recorder.hpp"

#include <iomanip>
#include <stdexcept>

namespace flightlab {
CsvRecorder::CsvRecorder(const std::filesystem::path& path) : stream_(path) {
  if (!stream_) throw std::runtime_error("could not open telemetry output: " + path.string());
  stream_ << "schema_version,sim_time_s,latitude_rad,longitude_rad,altitude_m,heading_rad,true_airspeed_mps\n";
  stream_ << std::setprecision(17);
}
void CsvRecorder::record(const AircraftState& s) {
  stream_ << "1," << s.sim_time_s << ',' << s.latitude_rad << ',' << s.longitude_rad << ','
          << s.altitude_m << ',' << s.heading_rad << ',' << s.true_airspeed_mps << '\n';
  if (!stream_) throw std::runtime_error("could not write telemetry");
}
} // namespace flightlab
