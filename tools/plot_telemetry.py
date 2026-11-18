#!/usr/bin/env python3
"""Render a compact, dependency-free SVG from a FlightLab telemetry CSV."""

import argparse
import csv
import html
from pathlib import Path


WIDTH, HEIGHT = 1280, 760
PLOT_LEFT, PLOT_RIGHT = 95, 1230
PANEL_HEIGHT, GAP = 170, 55
COLORS = {"actual": "#0b566e", "setpoint": "#e5652c", "grid": "#d8d0c0", "ink": "#152737"}


def scale(value, low, high, start, end):
    if high <= low:
        return (start + end) / 2
    return end - (value - low) * (end - start) / (high - low)


def path(points, x_key, y_key, x_range, y_range, y_top, y_bottom):
    commands = []
    for index, row in enumerate(points):
        x = scale(float(row[x_key]), *x_range, PLOT_LEFT, PLOT_RIGHT)
        y = scale(float(row[y_key]), *y_range, y_top, y_bottom)
        commands.append(("M" if index == 0 else "L") + f"{x:.2f},{y:.2f}")
    return " ".join(commands)


def bounds(points, *fields):
    values = [float(row[field]) for row in points for field in fields]
    low, high = min(values), max(values)
    padding = max((high - low) * 0.12, 0.1)
    return low - padding, high + padding


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("input", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    with args.input.open(newline="") as source:
        points = list(csv.DictReader(source))
    if not points:
        raise SystemExit("telemetry has no rows")
    x_range = (float(points[0]["sim_time_s"]), float(points[-1]["sim_time_s"]))
    panels = [
        ("Heading", "heading_rad", "heading_setpoint_rad", "rad", bounds(points, "heading_rad", "heading_setpoint_rad")),
        ("Altitude", "altitude_m", "altitude_setpoint_m", "m", bounds(points, "altitude_m", "altitude_setpoint_m")),
        ("True airspeed", "true_airspeed_mps", "airspeed_setpoint_mps", "m/s", bounds(points, "true_airspeed_mps", "airspeed_setpoint_mps")),
    ]
    lines = [
        f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {WIDTH} {HEIGHT}" role="img" aria-labelledby="title desc">',
        '<title id="title">FlightLab telemetry report</title>',
        '<desc id="desc">Measured state and setpoint traces from an included simulation run.</desc>',
        f'<rect width="{WIDTH}" height="{HEIGHT}" fill="#f7f1e5"/>',
        '<text x="50" y="48" fill="#152737" font-family="Arial, sans-serif" font-size="26" font-weight="700">FlightLab / telemetry record</text>',
        f'<text x="50" y="75" fill="#56636a" font-family="Arial, sans-serif" font-size="14">{html.escape(points[0]["backend"])} · {html.escape(points[0]["scenario"])} · {len(points)} exported samples</text>',
    ]
    for index, (label, actual, target, unit, y_range) in enumerate(panels):
        top = 120 + index * (PANEL_HEIGHT + GAP)
        bottom = top + PANEL_HEIGHT
        lines.extend([
            f'<text x="50" y="{top + 8}" fill="#152737" font-family="Arial, sans-serif" font-size="17" font-weight="700">{label} <tspan fill="#56636a" font-weight="400">[{unit}]</tspan></text>',
            f'<line x1="{PLOT_LEFT}" y1="{top}" x2="{PLOT_RIGHT}" y2="{top}" stroke="{COLORS["grid"]}"/>',
            f'<line x1="{PLOT_LEFT}" y1="{(top + bottom) / 2}" x2="{PLOT_RIGHT}" y2="{(top + bottom) / 2}" stroke="{COLORS["grid"]}" stroke-dasharray="4 5"/>',
            f'<line x1="{PLOT_LEFT}" y1="{bottom}" x2="{PLOT_RIGHT}" y2="{bottom}" stroke="{COLORS["grid"]}"/>',
            f'<text x="{PLOT_LEFT - 10}" y="{top + 5}" text-anchor="end" fill="#56636a" font-family="Arial, sans-serif" font-size="12">{y_range[1]:.2f}</text>',
            f'<text x="{PLOT_LEFT - 10}" y="{bottom}" text-anchor="end" fill="#56636a" font-family="Arial, sans-serif" font-size="12">{y_range[0]:.2f}</text>',
            f'<path d="{path(points, "sim_time_s", target, x_range, y_range, top, bottom)}" fill="none" stroke="{COLORS["setpoint"]}" stroke-width="1.6" stroke-dasharray="6 5"/>',
            f'<path d="{path(points, "sim_time_s", actual, x_range, y_range, top, bottom)}" fill="none" stroke="{COLORS["actual"]}" stroke-width="2.4"/>',
        ])
    unique_events = []
    for row in points:
        event = row["event"]
        if event not in unique_events:
            unique_events.append(event)
            x = scale(float(row["sim_time_s"]), *x_range, PLOT_LEFT, PLOT_RIGHT)
            lines.append(f'<line x1="{x:.2f}" y1="105" x2="{x:.2f}" y2="{HEIGHT - 55}" stroke="#e5652c" stroke-opacity="0.35"/>')
            lines.append(f'<text x="{x + 4:.2f}" y="100" fill="#a64218" font-family="Arial, sans-serif" font-size="11">{html.escape(event)}</text>')
    lines.append(f'<text x="{PLOT_LEFT}" y="{HEIGHT - 20}" fill="#56636a" font-family="Arial, sans-serif" font-size="12">time [s]</text>')
    lines.append('</svg>')
    args.output.write_text("\n".join(lines))


if __name__ == "__main__":
    main()
