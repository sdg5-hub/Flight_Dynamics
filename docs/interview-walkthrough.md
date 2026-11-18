# Interview walkthrough and resume evidence

## Explain one run

The named synthetic scenario starts from 900 m, northbound, at 55 m/s. The runner advances fixed 10 ms steps. At each step, the cascaded controller calculates a wrapped heading error, maps heading to bank and bank to aileron, maps altitude to pitch and pitch to elevator, and maps airspeed error to a throttle correction. Every PID has an explicit step, output clamp, derivative-on-measurement, and back-calculation anti-windup path. The backend rate-limits actuators, advances the intentionally synthetic state model, and exports every fifth step.

The biggest design tradeoff is fidelity versus inspectability. The synthetic backend is simple enough to reason about, test deterministically, and demonstrate controller plumbing. That simplicity is also why it cannot support aircraft-fidelity claims. The separate JSBSim adapter gives the next integration path, but the controller must not be connected until control-system ownership and model-specific properties are documented.

## Resume bullets — use measured values only

- Developed a C++20 deterministic flight-control experiment harness with cascaded PID control, versioned telemetry, browser replay, and CTest verification; measured **[replace with current run scope and metric]** across **[N]** seeded scenarios.
- Integrated pinned JSBSim **[version/model]** for a reproducible open-loop trim reference with explicit SI conversion and integration smoke tests; do not claim FlightLab controller performance against JSBSim until that scenario exists.

## Five interview prompts

1. Why does heading error need an angular wrap, and where is it applied?
2. What is derivative-on-measurement protecting against in this controller?
3. Why does the dashboard not count as numerical validation?
4. What differs between true airspeed and ground speed in the crosswind segment?
5. What must be established before connecting the FlightLab controller to a JSBSim model?
