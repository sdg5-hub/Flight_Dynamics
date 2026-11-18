#include <filesystem>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

#include "flightlab/csv_recorder.hpp"
#include "flightlab/metrics.hpp"
#include "flightlab/scenario_runner.hpp"
#include "flightlab/trainer_dynamics_backend.hpp"
#ifdef FLIGHTLAB_HAS_JSBSIM
#include "flightlab/jsbsim_backend.hpp"
#endif

namespace {
struct Options {
  std::string backend{"synthetic"};
  std::filesystem::path output{"results/nominal.csv"};
};

Options parse_options(const int argc, char** argv) {
  Options options;
  for (int index = 1; index < argc; ++index) {
    const std::string argument = argv[index];
    if (argument == "--backend" && index + 1 < argc) options.backend = argv[++index];
    else if (argument == "--output" && index + 1 < argc) options.output = argv[++index];
    else if (argument == "--help") {
      std::cout << "Usage: flightlab_run [--backend synthetic|jsbsim] [--output results/run.csv]\n";
      std::exit(0);
    } else throw std::invalid_argument("unknown or incomplete option: " + argument);
  }
  return options;
}

std::filesystem::path summary_path_for(const std::filesystem::path& csv_path) {
  auto summary = csv_path;
  summary.replace_extension(".summary.json");
  return summary;
}
}

int main(const int argc, char** argv) {
  try {
    const Options options = parse_options(argc, argv);
    if (!options.output.parent_path().empty()) std::filesystem::create_directories(options.output.parent_path());
    flightlab::CsvRecorder recorder(options.output);
    if (options.backend == "synthetic") {
      flightlab::TrainerDynamicsBackend backend;
      backend.reset({.latitude_rad = 0.5, .longitude_rad = 0.0, .altitude_m = 900.0,
                     .heading_rad = 0.0, .true_airspeed_mps = 55.0, .groundspeed_mps = 55.0});
      const auto result = flightlab::run_synthetic_autopilot_scenario(
          backend, {}, [&recorder](const flightlab::TelemetrySample& sample) { recorder.record(sample); });
      const auto metrics = flightlab::evaluate(result.samples_for_metrics, result.completed, result.termination_reason);
      const auto summary = summary_path_for(options.output);
      flightlab::write_summary_json(summary, metrics, std::string(flightlab::kSyntheticModelLabel),
                                    "heading-altitude-airspeed",
                                    "Synthetic reduced-order trainer; coefficients are not C172 data.");
      std::cout << "backend=" << flightlab::kSyntheticModelLabel << " samples=" << metrics.samples
                << " completed=" << metrics.completed << " heading_rms_deg=" << metrics.heading_rms_deg
                << " altitude_rms_m=" << metrics.altitude_rms_m << " output=" << options.output
                << " summary=" << summary << '\n';
      return metrics.completed ? 0 : 1;
    }
#ifdef FLIGHTLAB_HAS_JSBSIM
    if (options.backend == "jsbsim") {
      flightlab::JsbsimBackend backend(FLIGHTLAB_DEFAULT_JSBSIM_ROOT);
      backend.reset({.latitude_rad = 0.5, .longitude_rad = 0.0, .altitude_m = 1200.0,
                     .heading_rad = 0.0, .true_airspeed_mps = 51.4444, .groundspeed_mps = 51.4444});
      const flightlab::ScenarioConfig config{.name = "open-loop-trim-reference", .timestep_s = 0.01,
                                              .duration_s = 10.0, .seed = 20261118};
      const auto result = flightlab::run_open_loop_scenario(
          backend, config, [&recorder](const flightlab::TelemetrySample& sample) { recorder.record(sample); });
      const auto metrics = flightlab::evaluate(result.samples_for_metrics, result.completed, result.termination_reason);
      const auto summary = summary_path_for(options.output);
      flightlab::write_summary_json(summary, metrics, "jsbsim-c172r", "open-loop-trim-reference",
                                    "Pinned JSBSim C172R open-loop trim reference; no FlightLab controller is active.");
      std::cout << "backend=jsbsim-c172r samples=" << metrics.samples << " completed=" << metrics.completed
                << " output=" << options.output << " summary=" << summary << '\n';
      return metrics.completed ? 0 : 1;
    }
#endif
    throw std::invalid_argument("backend must be 'synthetic'"
#ifdef FLIGHTLAB_HAS_JSBSIM
                                " or 'jsbsim'"
#endif
    );
  } catch (const std::exception& error) {
    std::cerr << "flightlab_run: " << error.what() << '\n';
    return 1;
  }
}
