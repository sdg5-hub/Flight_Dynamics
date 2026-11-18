const preferredSources = [
  "../results/nominal.csv",
  "../results/flightlab-nominal.csv",
  "../results/jsbsim-open-loop.csv",
  "../results/steady-flight.csv",
];

const $ = (selector) => document.querySelector(selector);
const ui = {
  title: $("#run-title"), state: $("#run-state"), source: $("#source-file"),
  sideStatus: $("#side-status"), sideStatusText: $("#side-status-text"),
  caption: $("#replay-caption"), play: $("#play-pause"), reset: $("#reset"),
  scrubber: $("#scrubber"), time: $("#time-readout"), speed: $("#speed"),
  events: $("#event-list"), cursor: $("#maneuver-cursor"), trace: $("#trace-canvas"),
  traceCaption: $("#trace-caption"), path: $("#path-canvas"), pathCaption: $("#path-caption"),
  attitude: $("#attitude-canvas"), file: $("#file-input"), announcer: $("#announcer"),
  telemetrySummary: $("#telemetry-summary tbody"),
  values: Object.fromEntries(["time", "altitude", "heading", "airspeed", "bank", "pitch"].map((key) => [key, $(`#value-${key}`)])),
  record: Object.fromEntries(["samples", "duration", "schema", "fields"].map((key) => [key, $(`#record-${key}`)])),
  modelNote: $("#model-note"),
};

let replay = { rows: [], fields: [], source: "", index: 0, playing: false, lastFrame: 0, elapsed: 0, events: [] };
const palette = { ink: "#102a43", soft: "#4f6475", rule: "#b8c1bf", paper: "#f4f0e7", cyan: "#167a9b", cyanText: "#075a75", green: "#24765e", greenText: "#155843", orange: "#df5a26", orangeText: "#a73b15", sky: "#cce6e4" };

function csvRows(text) {
  const lines = text.replace(/^\uFEFF/, "").split(/\r?\n/).filter((line) => line.trim());
  if (lines.length < 2) throw new Error("The file needs a header row and at least one telemetry sample.");
  const split = (line) => {
    const cells = []; let cell = ""; let quoted = false;
    for (let i = 0; i < line.length; i += 1) {
      const char = line[i];
      if (char === '"' && line[i + 1] === '"') { cell += '"'; i += 1; }
      else if (char === '"') quoted = !quoted;
      else if (char === "," && !quoted) { cells.push(cell.trim()); cell = ""; }
      else cell += char;
    }
    cells.push(cell.trim()); return cells;
  };
  const fields = split(lines[0]).map((field) => field.toLowerCase().replace(/\s+/g, "_"));
  const rows = lines.slice(1).map((line) => {
    const cells = split(line); const row = {};
    fields.forEach((field, index) => { row[field] = cells[index] ?? ""; });
    return row;
  }).filter((row) => Object.values(row).some(Boolean));
  return { fields, rows };
}

function column(row, candidates) {
  for (const key of candidates) {
    if (key in row && row[key] !== "") { const n = Number(row[key]); return Number.isFinite(n) ? n : Number.NaN; }
  }
  return Number.NaN;
}
function sample(row) {
  return {
    t: column(row, ["sim_time_s", "time_s", "time", "timestamp_s", "timestamp"]),
    lat: column(row, ["latitude_rad", "lat_rad", "latitude"]), lon: column(row, ["longitude_rad", "lon_rad", "longitude"]),
    north: column(row, ["north_m", "north_position_m", "position_north_m"]), east: column(row, ["east_m", "east_position_m", "position_east_m"]),
    altitude: column(row, ["altitude_m", "altitude_asl_m", "h_sl_m", "alt_m"]),
    heading: column(row, ["heading_rad", "psi_rad", "yaw_rad", "heading"]),
    airspeed: column(row, ["true_airspeed_mps", "airspeed_mps", "tas_mps", "vtrue_mps"]),
    bank: column(row, ["bank_rad", "phi_rad", "roll_rad"]), pitch: column(row, ["pitch_rad", "theta_rad"]),
    headingSetpoint: column(row, ["heading_setpoint_rad", "heading_cmd_rad", "heading_target_rad"]),
    altitudeSetpoint: column(row, ["altitude_setpoint_m", "altitude_cmd_m", "altitude_target_m"]),
    airspeedSetpoint: column(row, ["airspeed_setpoint_mps", "tas_setpoint_mps", "airspeed_target_mps"]),
    event: row.event ?? row.mode_transition ?? row.mode ?? "",
  };
}
function radToDeg(value) { return Number.isFinite(value) ? value * 180 / Math.PI : Number.NaN; }
function fixed(value, unit, decimals = 1) { return Number.isFinite(value) ? `${value.toFixed(decimals)} ${unit}` : "N/A"; }
function heading(value) { return Number.isFinite(value) ? `${(((radToDeg(value) % 360) + 360) % 360).toFixed(1)}°` : "N/A"; }
function time(value) { return Number.isFinite(value) ? `${value.toFixed(2)} s` : "N/A"; }
function nowSample() { return replay.rows[replay.index] ?? null; }
function hasData() { return replay.rows.length > 0; }

function setStatus(kind, title, detail) {
  ui.state.textContent = title; ui.source.textContent = detail; ui.sideStatusText.textContent = title;
  ui.sideStatus.className = `status-dot ${kind === "ready" ? "is-ready" : kind === "error" ? "is-error" : ""}`;
}
function enableControls(enabled) {
  [ui.play, ui.reset, ui.scrubber, ui.speed].forEach((control) => { control.disabled = !enabled; });
}

function buildEvents() {
  const notable = [];
  let lastEvent = "";
  replay.rows.forEach((row, index) => {
    const label = String(row.event || "").trim();
    if (label && label !== lastEvent) notable.push({ index, label });
    lastEvent = label;
  });
  if (!notable.length && replay.rows.length) {
    notable.push({ index: 0, label: "Run start" });
    const thirds = [0.25, 0.5, 0.75].map((fraction) => Math.round((replay.rows.length - 1) * fraction));
    thirds.forEach((index, i) => notable.push({ index, label: ["Quarter run", "Mid-run", "Three-quarter run"][i] }));
    notable.push({ index: replay.rows.length - 1, label: "Run end" });
  }
  replay.events = notable.filter((item, index, items) => index === 0 || item.index !== items[index - 1].index);
  ui.events.replaceChildren(...replay.events.map((event) => {
    const item = document.createElement("li"); item.dataset.index = String(event.index);
    const stamp = document.createElement("time"); stamp.textContent = time(replay.rows[event.index].t);
    item.append(stamp, document.createTextNode(event.label)); return item;
  }));
}

function numericExtent(values) {
  const valid = values.filter(Number.isFinite); if (!valid.length) return null;
  let min = Math.min(...valid); let max = Math.max(...valid);
  if (min === max) { const pad = Math.max(Math.abs(min) * .1, 1); min -= pad; max += pad; }
  const pad = (max - min) * .09; return [min - pad, max + pad];
}
function scaleCanvas(canvas) {
  const ratio = window.devicePixelRatio || 1; const rect = canvas.getBoundingClientRect();
  const width = Math.max(1, Math.round(rect.width * ratio)); const height = Math.max(1, Math.round(rect.height * ratio));
  if (canvas.width !== width || canvas.height !== height) { canvas.width = width; canvas.height = height; }
  const context = canvas.getContext("2d"); context.setTransform(ratio, 0, 0, ratio, 0, 0); return { context, width: rect.width, height: rect.height };
}
function plotSeries(context, area, values, domain, extent, color, dashed = false) {
  if (!extent) return;
  context.save(); context.strokeStyle = color; context.lineWidth = dashed ? 1.2 : 1.8; context.setLineDash(dashed ? [5, 4] : []); context.beginPath(); let started = false;
  values.forEach((value, index) => {
    if (!Number.isFinite(value)) return;
    const x = area.x + ((replay.rows[index].t - domain[0]) / (domain[1] - domain[0] || 1)) * area.width;
    const y = area.y + area.height - ((value - extent[0]) / (extent[1] - extent[0])) * area.height;
    if (!started) { context.moveTo(x, y); started = true; } else context.lineTo(x, y);
  }); context.stroke(); context.restore();
}
function traceLabel(context, text, extent, color, x, y) {
  context.fillStyle = color; context.font = "700 11px ui-sans-serif, system-ui"; context.fillText(text, x, y);
  context.fillStyle = palette.soft; context.font = "11px ui-sans-serif, system-ui"; context.fillText(`${extent[1].toFixed(1)} / ${extent[0].toFixed(1)}`, x + 72, y);
}
function populateTelemetrySummary() {
  const definitions = [
    { label: "Heading", values: replay.rows.map((row) => radToDeg(row.heading)), targets: replay.rows.map((row) => radToDeg(row.headingSetpoint)), unit: "°" },
    { label: "Altitude", values: replay.rows.map((row) => row.altitude), targets: replay.rows.map((row) => row.altitudeSetpoint), unit: "m" },
    { label: "True airspeed", values: replay.rows.map((row) => row.airspeed), targets: replay.rows.map((row) => row.airspeedSetpoint), unit: "m/s" },
  ];
  const value = (number, unit) => Number.isFinite(number) ? `${number.toFixed(1)} ${unit}` : "N/A";
  ui.telemetrySummary.replaceChildren(...definitions.map((definition) => {
    const finite = definition.values.filter(Number.isFinite); const targets = definition.targets.filter(Number.isFinite);
    const cells = [definition.label, value(finite[0], definition.unit), value(finite.at(-1), definition.unit), finite.length ? `${value(Math.min(...finite), definition.unit)} to ${value(Math.max(...finite), definition.unit)}` : "N/A", value(targets.at(-1), definition.unit)];
    const row = document.createElement("tr"); row.replaceChildren(...cells.map((content, index) => { const cell = document.createElement(index ? "td" : "th"); if (!index) cell.scope = "row"; cell.textContent = content; return cell; })); return row;
  }));
}
function drawTrace() {
  const { context, width, height } = scaleCanvas(ui.trace); context.clearRect(0, 0, width, height); context.fillStyle = palette.paper; context.fillRect(0, 0, width, height);
  if (!hasData()) { ui.trace.setAttribute("aria-label", "No recorded telemetry response trace is available."); emptyCanvas(context, width, height, "Load telemetry to draw the response trace."); return; }
  const padding = { left: 14, right: 12, top: 17, bottom: 24 }; const domain = [replay.rows[0].t, replay.rows.at(-1).t]; const chartHeight = (height - padding.top - padding.bottom) / 3;
  const plots = [
    { label: "Heading °", values: replay.rows.map((row) => radToDeg(row.heading)), target: replay.rows.map((row) => radToDeg(row.headingSetpoint)), color: palette.cyan, labelColor: palette.cyanText },
    { label: "Altitude m", values: replay.rows.map((row) => row.altitude), target: replay.rows.map((row) => row.altitudeSetpoint), color: palette.green, labelColor: palette.greenText },
    { label: "TAS m/s", values: replay.rows.map((row) => row.airspeed), target: replay.rows.map((row) => row.airspeedSetpoint), color: palette.orange, labelColor: palette.orangeText },
  ];
  plots.forEach((plot, index) => {
    const area = { x: padding.left, y: padding.top + chartHeight * index + 10, width: width - padding.left - padding.right, height: chartHeight - 23 };
    context.strokeStyle = palette.rule; context.lineWidth = 1; context.beginPath(); context.moveTo(area.x, area.y + area.height); context.lineTo(area.x + area.width, area.y + area.height); context.stroke();
    const extent = numericExtent([...plot.values, ...plot.target]);
    plotSeries(context, area, plot.target, domain, extent, palette.ink, true);
    plotSeries(context, area, plot.values, domain, extent, plot.color);
    if (extent) traceLabel(context, plot.label, extent, plot.labelColor, area.x, area.y - 2);
    const current = nowSample(); const x = area.x + ((current.t - domain[0]) / (domain[1] - domain[0] || 1)) * area.width;
    context.strokeStyle = palette.orange; context.lineWidth = 1; context.beginPath(); context.moveTo(x, area.y); context.lineTo(x, area.y + area.height); context.stroke();
  });
  context.fillStyle = palette.soft; context.font = "11px ui-sans-serif, system-ui"; context.fillText(`${domain[0].toFixed(1)} s`, padding.left, height - 7); const end = `${domain[1].toFixed(1)} s`; context.fillText(end, width - padding.right - context.measureText(end).width, height - 7);
  ui.trace.setAttribute("aria-label", `Response trace with ${replay.rows.length} samples from ${domain[0].toFixed(1)} to ${domain[1].toFixed(1)} seconds; current time ${nowSample().t.toFixed(2)} seconds.`);
}
function positions() {
  const first = replay.rows[0]; const cosLat = Number.isFinite(first.lat) ? Math.cos(first.lat) : 1;
  return replay.rows.map((row) => {
    if (Number.isFinite(row.north) && Number.isFinite(row.east)) return [row.east, row.north];
    if (Number.isFinite(row.lat) && Number.isFinite(row.lon) && Number.isFinite(first.lat) && Number.isFinite(first.lon)) return [(row.lon - first.lon) * 6371000 * cosLat, (row.lat - first.lat) * 6371000];
    return [Number.NaN, Number.NaN];
  });
}
function drawPath() {
  const { context, width, height } = scaleCanvas(ui.path); context.clearRect(0, 0, width, height); context.fillStyle = palette.paper; context.fillRect(0, 0, width, height);
  const points = hasData() ? positions() : []; const valid = points.filter(([x, y]) => Number.isFinite(x) && Number.isFinite(y));
  if (valid.length < 2) { emptyCanvas(context, width, height, "No usable recorded position fields."); ui.pathCaption.textContent = "Position fields were not found in this telemetry file."; return; }
  const [minX, maxX] = numericExtent(valid.map(([x]) => x)); const [minY, maxY] = numericExtent(valid.map(([, y]) => y)); const pad = 24;
  const project = ([x, y]) => [pad + ((x - minX) / (maxX - minX)) * (width - pad * 2), height - pad - ((y - minY) / (maxY - minY)) * (height - pad * 2)];
  context.strokeStyle = palette.rule; context.lineWidth = 1; for (let i = 1; i < 4; i += 1) { const x = pad + (width - pad * 2) * i / 4; const y = pad + (height - pad * 2) * i / 4; context.beginPath(); context.moveTo(x, pad); context.lineTo(x, height - pad); context.moveTo(pad, y); context.lineTo(width - pad, y); context.stroke(); }
  context.strokeStyle = palette.cyan; context.lineWidth = 2.2; context.beginPath(); points.forEach((point, index) => { if (!Number.isFinite(point[0])) return; const [x, y] = project(point); if (index === 0) context.moveTo(x, y); else context.lineTo(x, y); }); context.stroke();
  const [startX, startY] = project(valid[0]); context.fillStyle = palette.green; context.beginPath(); context.arc(startX, startY, 4, 0, Math.PI * 2); context.fill();
  const active = points[replay.index]; if (active && Number.isFinite(active[0])) { const [x, y] = project(active); context.fillStyle = palette.orange; context.beginPath(); context.arc(x, y, 5.5, 0, Math.PI * 2); context.fill(); }
  context.fillStyle = palette.soft; context.font = "11px ui-sans-serif, system-ui"; context.fillText("E / projected metres", pad, height - 5); context.save(); context.translate(10, height - pad); context.rotate(-Math.PI / 2); context.fillText("N / projected metres", 0, 0); context.restore();
  ui.pathCaption.textContent = "Projected from recorded latitude/longitude or north/east fields. Green marks the first sample; orange marks the active replay sample.";
}
function drawAttitude() {
  const { context, width, height } = scaleCanvas(ui.attitude); context.clearRect(0, 0, width, height); const current = nowSample();
  context.fillStyle = palette.paper; context.fillRect(0, 0, width, height); if (!current || (!Number.isFinite(current.bank) && !Number.isFinite(current.pitch))) { emptyCanvas(context, width, height, "Bank/pitch unavailable"); return; }
  const cx = width / 2; const cy = height / 2; const radius = Math.min(width, height) * .39; const bank = current.bank || 0; const pitch = current.pitch || 0;
  context.save(); context.beginPath(); context.arc(cx, cy, radius, 0, Math.PI * 2); context.clip(); context.translate(cx, cy); context.rotate(-bank); context.translate(0, Math.max(-radius * .55, Math.min(radius * .55, pitch * radius * 1.9)));
  context.fillStyle = palette.sky; context.fillRect(-radius * 1.5, -radius * 1.5, radius * 3, radius * 1.5); context.fillStyle = "#d9cab0"; context.fillRect(-radius * 1.5, 0, radius * 3, radius * 1.5);
  context.strokeStyle = palette.ink; context.lineWidth = 1.4; context.beginPath(); context.moveTo(-radius * 1.3, 0); context.lineTo(radius * 1.3, 0); context.stroke(); context.restore();
  context.strokeStyle = palette.ink; context.lineWidth = 1.5; context.beginPath(); context.arc(cx, cy, radius, 0, Math.PI * 2); context.stroke(); context.beginPath(); context.moveTo(cx - radius * .36, cy); context.lineTo(cx + radius * .36, cy); context.moveTo(cx, cy - 7); context.lineTo(cx, cy + 7); context.stroke();
  context.fillStyle = palette.ink; context.font = "700 11px ui-sans-serif, system-ui"; context.textAlign = "center"; context.fillText(`BANK ${fixed(radToDeg(current.bank), "°")}`, cx, height - 14); context.fillText(`PITCH ${fixed(radToDeg(current.pitch), "°")}`, cx, 18); context.textAlign = "start";
}
function emptyCanvas(context, width, height, message) { context.fillStyle = palette.soft; context.font = "13px ui-sans-serif, system-ui"; context.textAlign = "center"; context.fillText(message, width / 2, height / 2); context.textAlign = "start"; }

function updateValues() {
  const current = nowSample(); if (!current) return;
  ui.values.time.textContent = time(current.t); ui.values.altitude.textContent = fixed(current.altitude, "m"); ui.values.heading.textContent = heading(current.heading); ui.values.airspeed.textContent = fixed(current.airspeed, "m/s"); ui.values.bank.textContent = fixed(radToDeg(current.bank), "°"); ui.values.pitch.textContent = fixed(radToDeg(current.pitch), "°");
  const max = Math.max(1, replay.rows.length - 1); ui.scrubber.value = String(replay.index); ui.time.value = time(current.t); ui.cursor.style.left = `${(replay.index / max) * 100}%`;
  ui.events.querySelectorAll("li").forEach((item) => item.classList.toggle("event-active", Number(item.dataset.index) <= replay.index && (!item.nextElementSibling || Number(item.nextElementSibling.dataset.index) > replay.index)));
}
function render() { updateValues(); drawTrace(); drawPath(); drawAttitude(); }
function clearTelemetry() {
  replay = { rows: [], fields: [], source: "", index: 0, playing: false, lastFrame: 0, elapsed: 0, events: [] };
  ui.title.textContent = "No run loaded"; ui.caption.textContent = "Load a generated telemetry file to inspect a recorded run.";
  ui.scrubber.max = "0"; ui.scrubber.value = "0"; ui.time.value = "N/A"; ui.events.replaceChildren();
  Object.values(ui.values).forEach((value) => { value.textContent = "N/A"; }); ui.cursor.style.left = "0%";
  ["samples", "duration", "schema", "fields"].forEach((key) => { ui.record[key].textContent = "N/A"; });
  ui.modelNote.textContent = "The dashboard will identify the telemetry source after a file loads. Consult the scenario and model documentation before treating any response as an engineering conclusion.";
  ui.traceCaption.textContent = "No recorded samples available."; ui.telemetrySummary.replaceChildren(); enableControls(false); render();
}
function reset() { replay.index = 0; replay.elapsed = replay.rows[0]?.t ?? 0; replay.playing = false; ui.play.textContent = "Play"; ui.play.setAttribute("aria-label", "Play replay"); render(); }
function tick(stamp) {
  if (!replay.playing || !hasData()) return;
  const delta = replay.lastFrame ? (stamp - replay.lastFrame) / 1000 : 0; replay.lastFrame = stamp;
  const start = replay.rows[0].t; const end = replay.rows.at(-1).t; replay.elapsed = Math.min(end, replay.elapsed + delta * Number(ui.speed.value));
  let index = replay.index; while (index < replay.rows.length - 1 && replay.rows[index + 1].t <= replay.elapsed) index += 1; replay.index = index; render();
  if (replay.elapsed >= end) { replay.playing = false; ui.play.textContent = "Replay"; ui.play.setAttribute("aria-label", "Replay from start"); ui.announcer.textContent = "Replay reached the final sample."; return; }
  requestAnimationFrame(tick);
}
function togglePlay() {
  if (!hasData()) return;
  if (replay.index === replay.rows.length - 1) { replay.index = 0; replay.elapsed = replay.rows[0].t; }
  replay.playing = !replay.playing; replay.lastFrame = 0; ui.play.textContent = replay.playing ? "Pause" : "Play"; ui.play.setAttribute("aria-label", replay.playing ? "Pause replay" : "Play replay"); if (replay.playing) requestAnimationFrame(tick);
}

function loadTelemetry(text, source) {
  const parsed = csvRows(text); const converted = parsed.rows.map(sample).filter((row) => Number.isFinite(row.t)).sort((a, b) => a.t - b.t);
  if (converted.length < 2) throw new Error("No finite time column was found. Expected sim_time_s, time_s, or time.");
  replay = { rows: converted, fields: parsed.fields, source, index: 0, playing: false, lastFrame: 0, elapsed: converted[0].t, events: [] };
  ui.title.textContent = source.replace(/^.*\//, "").replace(/[-_]/g, " ").replace(/\.csv$/i, ""); ui.caption.textContent = "The replay pointer follows the file’s actual sample times; it does not resimulate the aircraft in the browser.";
  setStatus("ready", "Replay ready", source); enableControls(true); ui.scrubber.max = String(converted.length - 1); buildEvents();
  const schema = parsed.fields.find((field) => field === "schema_version") ? String(parsed.rows[0].schema_version || "declared") : "CSV headers";
  ui.record.samples.textContent = String(converted.length); ui.record.duration.textContent = time(converted.at(-1).t - converted[0].t); ui.record.schema.textContent = schema; ui.record.fields.textContent = `${parsed.fields.length} fields`; ui.modelNote.textContent = `Replaying ${source.replace(/^.*\//, "")}. The dashboard uses only fields present in the CSV. It does not validate a model, infer trim quality, or establish aircraft fidelity; read the scenario and backend documentation with the run.`;
  ui.traceCaption.textContent = `${converted.length.toLocaleString()} recorded samples from ${time(converted[0].t)} to ${time(converted.at(-1).t)}. Orange is the current replay position.`; populateTelemetrySummary();
  reset(); ui.announcer.textContent = `Loaded ${converted.length} telemetry samples from ${source}.`;
}
async function loadDefault() {
  for (const source of preferredSources) {
    try { const response = await fetch(source, { cache: "no-store" }); if (!response.ok) continue; loadTelemetry(await response.text(), source); return; } catch { /* try the next generated output */ }
  }
  clearTelemetry(); setStatus("waiting", "Waiting for telemetry", "Run the simulator, then reload this page"); ui.modelNote.textContent = "No generated CSV was found at the expected results paths. Run the simulator and serve the repository root, or use “Open local CSV” to inspect an output file.";
}

ui.play.addEventListener("click", togglePlay); ui.reset.addEventListener("click", reset); ui.scrubber.addEventListener("input", () => { if (!hasData()) return; replay.index = Number(ui.scrubber.value); replay.elapsed = replay.rows[replay.index].t; render(); });
ui.file.addEventListener("change", async () => { const file = ui.file.files?.[0]; if (!file) return; try { loadTelemetry(await file.text(), file.name); } catch (error) { clearTelemetry(); setStatus("error", "Could not load telemetry", error.message); ui.announcer.textContent = `Could not load CSV: ${error.message}`; } });
window.addEventListener("resize", () => render()); loadDefault();
