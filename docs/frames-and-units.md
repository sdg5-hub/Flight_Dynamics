# Frames and units

The public API uses SI units. `AircraftState` currently stores geodetic latitude and longitude in radians, altitude in metres, true airspeed in metres per second, and heading in radians. Heading is navigation heading, clockwise from north. It is neither yaw, course, nor ground track.

The fallback assumes zero wind, so true airspeed equals ground speed. It defines no body frame, quaternion convention, or angular-rate convention. Those contracts must be documented with the JSBSim adapter before any attitude/controller work is added.
