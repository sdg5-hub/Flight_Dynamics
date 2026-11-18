# Implementation plan

## Evidence-led scope

The first useful deliverable is a deterministic, headless runner with genuine telemetry and tests. A real JSBSim backend is required before FlightLab is described as an aircraft simulator. The fallback backend exists only to exercise the common interfaces while that dependency is unavailable.

## Milestone sequence

1. **M0 — foundation (complete):** C++20/CMake project, typed backend seam, fixed-step scenario runner, input validation, CSV schema, and unit checks.
2. **M1 — JSBSim vertical slice:** verify/pin JSBSim and a civilian trainer model; implement explicit unit conversion, reproducible trim, a real open-loop run, integration test, and plot from actual log.
3. **M2 — controls:** sensors, actuator limits, discrete PID, conservative heading/altitude/airspeed scenarios, and controller tests.
4. **M3 — evidence:** disturbance and delay experiments, acceptance configuration, replay dashboard, CI, and requirements/test mapping.
5. **M4+ — optional:** educational 6-DOF backend, then one clearly scoped advanced comparison.

## Numerical contract today

The runner executes a positive fixed timestep and rejects nonfinite state. Time is simulated time and is independent of wall time. The fallback uses a short-step spherical-Earth position update with no wind and holds altitude, heading, and airspeed fixed. It must not be used for control conclusions.
