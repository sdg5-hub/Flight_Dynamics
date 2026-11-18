# Flight_Dynamics / FlightLab

FlightLab is a reproducible, headless C++20 harness for evaluating civilian-aircraft simulation and autopilot software. It is an early portfolio project: no real-flight, certification, hardware-in-the-loop, or real-time claims are made.

## Current state

M0 is complete. The repository builds and tests a deterministic scenario runner and versioned CSV telemetry path. JSBSim is **not installed** on this machine, so M1's actual aircraft-model integration has not been performed. The executable currently uses `SteadyFlightBackend`, a deliberately simple constant-altitude, constant-heading kinematic fallback that validates plumbing only; it is not an aerodynamic or 6-DOF model.

## Build and run

```sh
cmake --preset default
cmake --build --preset default
ctest --preset default
./build/default/flightlab_run results/steady-flight.csv
```

The output CSV uses schema version 1. `sim_time_s`, angles, altitude, and airspeed are SI-base units (radians, metres, metres per second).

## Architecture

```text
ScenarioConfig -> deterministic ScenarioRunner -> DynamicsBackend -> AircraftState
                                              -> CsvRecorder -> telemetry CSV
```

`DynamicsBackend` is the narrow seam for a future pinned JSBSim adapter. Disk I/O happens through the observer callback, not inside backend integration.

## Limitations and next step

There is no controller, sensor model, actuator model, trim, wind, visualization, or JSBSim integration yet. Install or otherwise provide a verified JSBSim version and a licensed, pinned trainer model before implementing the adapter. See `docs/continuation.md`.
