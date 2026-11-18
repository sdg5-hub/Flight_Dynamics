# JSBSim integration

The repository includes JSBSim as a pinned submodule at v1.3.3 (`66f7fafc07fe13e7ede9db20873030fed2881713`). Clone with:

```sh
git clone --recurse-submodules https://github.com/sdg5-hub/Flight_Dynamics.git
```

The adapter uses the verified public `FGFDMExec` workflow: configure data paths, load `c172r`, set initial conditions, call `RunIC`, execute `FGTrim::DoTrim`, then call `Run` at a fixed step. It checks model loading, initial-condition execution, trim, fixed-step consistency, run termination, and finite output.

JSBSim exports source properties in its native units. `src/backends/jsbsim_backend.cpp` explicitly converts altitude feet → metres and true/ground speed feet per second → metres per second. Latitude, longitude, attitude, and headings are read in radians. Control property names are model-specific and documented in code for the pinned C172R model.

References: [JSBSim source](https://github.com/JSBSim-Team/jsbsim), [reference manual](https://jsbsim-team.github.io/jsbsim-reference-manual/).
