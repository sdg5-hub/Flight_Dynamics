# Requirements-to-test map

| Requirement | Evidence | Automated check |
| --- | --- | --- |
| Circular heading errors stay bounded | `wrap_angle_rad` | `test_angles` |
| PID output honors bounds under persistent error | derivative-on-measurement PID + back-calculation | `test_pid_saturation` |
| Fixed-step synthetic replay is deterministic | same input/config produces equal final state | `test_synthetic_determinism` |
| Invalid integration inputs fail explicitly | backend validates time step | `test_invalid_timestep` |
| Selected JSBSim model loads, trims, and advances | `JsbsimBackend` C172R smoke integration | `test_jsbsim_trim_smoke` when JSBSim is enabled |
| Recorded runs expose model, event, setpoint, state, controls, and saturation | CSV schema v2 | `flightlab_run` artifacts and dashboard parser |
| Frozen synthetic design criteria are enforced | versioned acceptance configuration | `tools/check_acceptance.py` in CI against the generated nominal summary |

The tests establish bounded software behavior, not aircraft fidelity or real-world safety.
