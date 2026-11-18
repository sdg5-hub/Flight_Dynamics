# Dashboard direction: flight-test record sheet

Mode: **Operate**. Seed reference: `seed:ff5b00af069f6e6b`.

The dashboard is a working local replay tool, not a decorative cockpit. Its visual system takes four things from a civilian engineering flight-test record: compact technical type, warm ivory/natural-ink palette, measured density, and one signature move—a test-card cursor that tracks the active maneuver through the data. Navigation, controls, keyboard behavior, and charts remain familiar web conventions.

The first viewport must answer: which run is being replayed, what model produced it, whether it completed, where the active state is, and how to inspect a maneuver. It leads with actual telemetry, labels synthetic data as such, and provides visible empty/loading/error states. Orange marks a selected maneuver or event; cyan and green distinguish data series; status never relies on color alone.

This direction was selected by the automated build workflow because no interactive design-decision channel was available in this session. It is a code-led implementation; no illustrative assets are used in place of evidence.
