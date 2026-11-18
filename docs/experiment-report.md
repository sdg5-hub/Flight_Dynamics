# Experiment report — baseline artifacts

Run date: 2026-11-18. Host: arm64 macOS 26.6.2, Apple Clang 21.0.0, CMake 4.5.0. The raw inputs to the report are committed beneath `results/`; regenerate them with the commands below rather than treating this page as a substitute for the data.

```sh
./build/default/flightlab_run --backend synthetic --output results/nominal.csv
python3 tools/plot_telemetry.py results/nominal.csv results/nominal.svg
./build/default/flightlab_run --backend jsbsim --output results/jsbsim-open-loop.csv
```

## Synthetic controller baseline

Scenario: `heading-altitude-airspeed`. The 90-second fixed-step run exported 1,801 samples at 20 Hz. It commands a 30° heading change, 120 m altitude change, 5 m/s airspeed change, and 12 m/s crosswind interval in the order recorded in the CSV events.

| Metric | Measured value | Definition |
| --- | ---: | --- |
| Completion | completed | termination reason `duration-complete` |
| Nonfinite state count | 0 | count across exported samples |
| Heading RMS error | 5.6226° | circular error, samples at or after 16 s |
| Altitude RMS error | 51.6082 m | samples at or after 38 s |
| Airspeed RMS error | 0.2009 m/s | samples at or after 63 s |
| Maximum bank | 21.6991° | maximum absolute exported bank |
| Saturation fraction | 0.0422 | any controller loop reported saturated |

The pre-recorded synthetic design targets are met by this run: heading RMS ≤ 8.0°, altitude RMS ≤ 65.0 m, airspeed RMS ≤ 0.5 m/s, and zero nonfinite states. These are initial controller-design criteria for this synthetic configuration, not certification gates or aircraft-performance claims.

These values describe the included synthetic model and its named configuration only. They should not be interpreted as Cessna, flight-test, or safety performance.

## JSBSim open-loop reference

Scenario: `open-loop-trim-reference`. The C172R model loaded, initialized, completed full trim, and advanced 10 seconds at 10 ms with a constant open-loop command. The exported reference contains 201 samples. FlightLab’s cascaded PID controller is not active in this run, so the controller-oriented RMS metrics are necessarily zero and are not an evaluation result.

## Interpretation notes

- Initial state and setpoint changes are preserved in the raw telemetry rather than trimmed out of plots or metrics.
- Heading error uses angular wrapping across ±π.
- The 20 Hz export cadence is an integer divisor of the 100 Hz integration step.
- The dashboard replays the CSV; it does not simulate, interpolate, or overwrite results.
- The current acceptance file holds initial synthetic-model design targets. It is not a certification or hardware-validation threshold set.
- `results/manifest.json` records exact commands, pinned JSBSim revision, host toolchain, and SHA-256 identities for the committed artifacts.
