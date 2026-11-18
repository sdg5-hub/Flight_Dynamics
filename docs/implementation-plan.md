# Implementation plan

## Completed vertical slices

- **M0:** C++20/CMake, typed backend seam, fixed-step runner, validation, versioned telemetry, and headless tests.
- **M1:** pinned JSBSim v1.3.3 source/model dependency; C172R load, airborne initialization, full trim, explicit unit conversion, 10 ms open-loop run, generated CSV/JSON, and integration smoke test.
- **M2 (synthetic-model scope):** discrete PID with derivative-on-measurement and back-calculation anti-windup; cascaded heading/bank, altitude/pitch, and airspeed/throttle loops; rate-limited actuators; named change/disturbance scenario.
- **M3 (initial evidence scope):** metric evaluator, standard-library SVG plotter, replay dashboard, requirements/test map, source/license inventory, and CI definition.

## Next milestone

Connect FlightLab’s controller to the JSBSim adapter only after writing a model-specific control contract: pinned properties, internal-system disablement, explicit sensor/actuator semantics, known trim point, and a separate regression scenario. Then add delayed/noisy sensor conditions, multiple seeds, fixed acceptance checks, and side-by-side synthetic/JSBSim comparison only where assumptions match.

## Numerical contract

The simulation executes a positive, fixed 10 ms timestep. Telemetry is exported on an integer five-step cadence. Simulated time never depends on browser, wall-clock, or file playback. Invalid/nonfinite inputs are rejected or terminate a run with an explicit reason.
