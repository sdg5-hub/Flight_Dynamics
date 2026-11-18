# Telemetry replay dashboard

The dashboard is a dependency-free static consumer of recorded CSV telemetry. It does not execute flight dynamics or create success data in the browser.

Serve the repository root after generating a run:

```sh
python3 -m http.server 8000
open http://127.0.0.1:8000/dashboard/
```

It looks for these generated artifacts in order:

1. `results/nominal.csv`
2. `results/flightlab-nominal.csv`
3. `results/jsbsim-open-loop.csv`
4. `results/steady-flight.csv`

Use **Open local CSV** to inspect another output without copying it into the repository. The minimum contract is a finite `sim_time_s`, `time_s`, or `time` column with two samples. The dashboard recognizes common named fields for latitude/longitude or north/east position, altitude, heading, true airspeed, bank, pitch, setpoints, and events when present. Missing fields display as `N/A`; they are never interpolated or invented.
