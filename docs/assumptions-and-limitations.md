# Assumptions and limitations

## Synthetic control experiment

The default `synthetic` run uses a deliberately reduced-order trainer model: first-order roll/pitch response, simplified energy response, rate-limited normalized actuators, spherical-Earth local position update, and an imposed crosswind. Its coefficients are educational parameters, not Cessna data. The controller observes perfect simulated state; no sensor noise, delay, estimator, mass variation, terrain, or airframe-limit model is included.

It is appropriate for demonstrating deterministic scheduling, PID behavior, saturation handling, telemetry, and reproducible scenario evaluation. It is not appropriate for flight-envelope, stability, certification, safety, or hardware claims.

## JSBSim reference experiment

The optional JSBSim path pins the upstream source to v1.3.3 / `66f7fafc07fe13e7ede9db20873030fed2881713` and loads the bundled `c172r` model. It applies explicit feet/metres and feet-per-second/metres-per-second conversions at the adapter boundary, initializes an airborne condition, runs full trim, then advances fixed 10 ms open-loop steps.

FlightLab does not claim that its selected upstream `c172r` model is a validated Cessna representation. Its internal autopilot is left off; the FlightLab controller is not yet connected to the JSBSim model.

## Reproducibility scope

Same-toolchain replay is tested for the synthetic backend. Cross-platform numerical tolerance and a full JSBSim controller regression suite remain future work. The local dashboard replays recorded CSV; it cannot alter the numerical result.
