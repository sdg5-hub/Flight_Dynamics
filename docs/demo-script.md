# Two-minute demo script

1. Start at the README and state the scope: FlightLab is a civilian, simulated control-software project with an explicitly synthetic trainer model and a separate JSBSim open-loop reference.
2. Run `ctest --preset default`, then explain that the suite covers angle wrapping, PID saturation, deterministic replay, invalid timestep rejection, and actual JSBSim load/trim/advance.
3. Run the synthetic scenario. Open `results/nominal.summary.json` and identify the scenario, metric windows, completion status, and zero nonfinite states.
4. Serve the repository and open `/dashboard/`. Scrub from the heading step to the altitude and airspeed steps. Show the current sample, response traces, path, attitude, and event timeline are all read from the CSV.
5. Point out `docs/assumptions-and-limitations.md`: the default coefficients are synthetic and the dashboard does not establish aircraft fidelity.
6. Run the JSBSim path separately. Explain the pinned model, trim, explicit conversion boundary, and that FlightLab’s controller is not yet connected to it.
7. Close with the next milestone: define control/property ownership, disable model-internal confounders, then add conservative FlightLab-controlled JSBSim scenarios.
