# Frames and units

FlightLab API and CSV schema v2 use SI units: radians, metres, metres per second, and seconds. `heading_rad` is navigation heading, clockwise from local north. It is not yaw, course, or ground track. `bank_rad` and `pitch_rad` are attitude Euler angles; the public interface intentionally does not expose a quaternion yet.

`north_m` and `east_m` are a local visualization frame whose origin is the scenario initial condition. In the synthetic backend they are integrated from air-relative velocity plus crosswind. In the JSBSim adapter they are a small-area projection from geocentric latitude/longitude relative to the initialized origin. They are for local replay, not a general geodesy library.

JSBSim property conversions happen only in `JsbsimBackend`:

| JSBSim property | JSBSim unit | FlightLab unit |
| --- | --- | --- |
| `position/h-sl-ft` | ft | m |
| `velocities/vtrue-fps`, `velocities/vg-fps` | ft/s | m/s |
| `position/lat-gc-rad`, `position/long-gc-rad` | rad | rad |
| `attitude/psi-rad`, `phi-rad`, `theta-rad` | rad | rad |

The synthetic backend assumes no vertical wind and applies crosswind only to ground-relative position; true airspeed remains air-relative. This is a scope decision, not a wind model.
