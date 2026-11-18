# Continuation state

Last completed milestone: **M3 initial evidence scope**. The next meaningful task is JSBSim/controller integration, not adding more UI.

## Validation on 2026-11-18

- Host: arm64 macOS 26.6.2; 8 GiB RAM.
- Tools: Apple Clang 21.0.0, CMake 4.5.0, Git 2.54.0, Python 3.14.7.
- JSBSim: upstream v1.3.3, commit `66f7fafc07fe13e7ede9db20873030fed2881713`, pinned submodule.
- Commands run: `cmake --preset default`, `cmake --build --preset default`, `ctest --preset default`, the no-JSBSim CMake configure/build/test path, and the synthetic/JSBSim evidence commands in the README.
- Checks: both configured test suites passed 1/1. The default suite included the JSBSim C172R load/trim/advance smoke test.

## Stored run evidence

`results/nominal.summary.json` records a completed synthetic scenario: 1,801 exported samples, 90 s duration, heading RMS 5.6226°, altitude RMS 51.6082 m, airspeed RMS 0.2009 m/s, maximum bank 21.6991°, saturation fraction 0.0422, and zero nonfinite states. Those values apply only to the named synthetic scenario and metric windows in `configs/acceptance.json`.

`results/jsbsim-open-loop.summary.json` records a completed 10 s, 201-sample open-loop C172R trim reference. It is not an autopilot-performance benchmark.

`results/manifest.json` captures the host/toolchain, exact generation commands, pinned JSBSim commit, and SHA-256 identities for the committed evidence artifacts.

## Exact next actions

1. Define FlightLab-to-C172R control/property ownership and disable any model-internal systems that would confound evaluation.
2. Add sensor and actuator contracts to both backends, then run FlightLab PID against a small, conservative JSBSim scenario.
3. Add test fixtures for telemetry parsing, metric-window boundaries, and JSBSim same-toolchain tolerance replay.
4. Add seeded delay/noise/disturbance experiments and report distributions against the frozen acceptance criteria.
