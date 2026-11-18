# FlightLab dashboard design

## Intent

The dashboard is a code-led civilian flight-test record sheet: a local instrument for reviewing recorded simulation evidence, rather than a cockpit imitation or a generic admin interface. It should answer which run is loaded, what produced it, where replay is positioned, and what the recorded response does before asking the reader to interpret it.

## Visual system

| Role | Token | Use |
| --- | --- | --- |
| Record paper | `#f4f0e7` | Main field and canvas ground |
| Measurement ink | `#102a43` | Typography, rules, rail, primary controls |
| Active/test marker | `#df5a26` / `#a73b15` | Replay cursor, selected state, actions, status emphasis |
| Heading trace | `#167a9b` | Recorded heading series |
| Altitude trace | `#24765e` | Recorded altitude series |
| Supporting text/rules | `#4f6475` / `#b8c1bf` | Secondary information and separators |

IBM Plex Sans is self-hosted in `dashboard/assets/`; use its compact, technical weight hierarchy. Large run titles use tight tracking. Borders and spacing establish rhythm; avoid glass surfaces, gradient decoration, oversized cards, and status that depends only on color.

## Layout and behavior

- Desktop uses a fixed deep-navy identity rail and a broad, rule-led evidence field.
- At 850 px and below, the rail becomes a compact top navigation; at 540 px, replay controls form a two-column grid and events wrap into a readable two-column record.
- The replay strip is the signature interaction: its orange cursor is tied to actual CSV sample time. Play, pause, reset, seek, speed, keyboard focus, and empty/error states remain conventional and obvious.
- The trace compares actual values to explicit dashed setpoints. Projected path, attitude, live values, event record, and provenance provide separate views of the same recorded run.

## Evidence and accessibility contract

- Browser code only parses and replays CSV; it never simulates, interpolates, or fabricates telemetry.
- The selected file, backend/model limitations, and the difference between replay and validation stay visible in copy.
- Controls have visible focus states and mobile-sized hit areas. Reduced-motion preferences suppress ornamental transitions.
- Canvases carry useful labels, and a visually-hidden semantic table exposes start, final, range, and final-setpoint values for the three response traces.
- A failed CSV import clears prior run information before reporting the error, so stale telemetry cannot be mistaken for the rejected file.

## Maintain

Keep future visual additions data-led, restrained, and clearly scoped to the model behind the loaded file. Do not add a 3D cockpit, live-looking invented gauges, unrelated illustration, or claims of aircraft fidelity without corresponding evidence.
