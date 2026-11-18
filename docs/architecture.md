# Architecture

```mermaid
flowchart LR
  scenario[Named fixed-step scenario] --> runner[Deterministic runner]
  runner --> autopilot[Cascaded PID controller]
  autopilot --> backend{DynamicsBackend}
  backend --> synthetic[Synthetic reduced-order trainer]
  backend --> jsbsim[Pinned JSBSim C172R adapter]
  backend --> state[Aircraft state + actuator state]
  state --> telemetry[CSV telemetry v2]
  telemetry --> metrics[Metric evaluator + JSON summary]
  telemetry --> plot[Standard-library SVG plotter]
  telemetry --> dashboard[Local browser replay]
```

The numerical path is single-threaded. Telemetry is exported every five 10 ms integration steps (20 Hz); the simulation itself runs at 100 Hz. Rendering and file playback never alter simulation time.

`DynamicsBackend` is the boundary between FlightLab’s scenario/control code and a model implementation. The synthetic trainer exists for an educational, inspectable controller experiment. The JSBSim adapter is a separate open-loop reference path with a real, pinned flight-dynamics model; it does not yet run FlightLab’s controller.
