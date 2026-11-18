#!/usr/bin/env python3
"""Check a FlightLab summary JSON against a frozen design-target file."""

from __future__ import annotations

import argparse
import json
import math
from pathlib import Path


def read_json(path: Path) -> dict[str, object]:
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as error:
        raise SystemExit(f"acceptance check: could not read {path}: {error}") from error
    if not isinstance(value, dict):
        raise SystemExit(f"acceptance check: {path} must contain a JSON object")
    return value


def numeric(value: object, label: str, failures: list[str]) -> float | None:
    if isinstance(value, bool) or not isinstance(value, (int, float)) or not math.isfinite(value):
        failures.append(f"{label} must be a finite number")
        return None
    return float(value)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("acceptance", type=Path, help="frozen design-target JSON")
    parser.add_argument("summary", type=Path, help="generated FlightLab summary JSON")
    args = parser.parse_args()
    acceptance = read_json(args.acceptance)
    summary = read_json(args.summary)
    failures: list[str] = []

    if summary.get("completed") is not True:
        failures.append(f"run did not complete (termination_reason={summary.get('termination_reason', 'missing')!r})")
    for key in ("expected_backend", "expected_scenario"):
        expected = acceptance.get(key)
        if expected is not None:
            summary_key = key.removeprefix("expected_")
            if summary.get(summary_key) != expected:
                failures.append(f"{summary_key} is {summary.get(summary_key)!r}, expected {expected!r}")

    targets = acceptance.get("targets")
    if not isinstance(targets, dict) or not targets:
        failures.append("acceptance targets must be a non-empty object")
    else:
        for target_name, target_value in targets.items():
            if not isinstance(target_name, str):
                failures.append("acceptance target names must be strings")
                continue
            bound = numeric(target_value, f"target {target_name}", failures)
            metric_name = target_name.removesuffix("_max")
            measured = numeric(summary.get(metric_name), f"summary metric {metric_name}", failures)
            if bound is None or measured is None:
                continue
            if target_name.endswith("_max"):
                if measured > bound:
                    failures.append(f"{metric_name}={measured:g} exceeds maximum {bound:g}")
            elif measured != bound:
                failures.append(f"{metric_name}={measured:g} does not equal required value {bound:g}")

    if failures:
        print("ACCEPTANCE CHECK FAILED")
        for failure in failures:
            print(f"- {failure}")
        return 1
    print(f"ACCEPTANCE CHECK PASSED: {args.summary} matches {args.acceptance}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
