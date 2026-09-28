#!/usr/bin/env python3
"""Read AlphaGlueRn222 decay phase-space binary files."""

from __future__ import annotations

import argparse
import csv
import glob
import os
import re
import sys
from pathlib import Path
from typing import Dict, List, Sequence, Tuple

import numpy as np


BASE_COLUMNS = [
    "x_mm",
    "y_mm",
    "z_mm",
    "dir_x",
    "dir_y",
    "dir_z",
    "energy_MeV",
    "event_id",
    "particle_id",
    "copy_no",
    "time_s",
    "origin_id",
    "world_x_mm",
    "world_y_mm",
    "world_z_mm",
    "sc_event_id",
]

EXTENDED_COLUMNS = BASE_COLUMNS + ["track_id", "parent_id"]


PARTICLE_ID_MAP: Dict[int, str] = {
    1: "e-",
    2: "gamma",
    3: "alpha",
    4: "Rn220",
    5: "Po216",
    6: "Pb212",
    7: "Bi212",
    8: "Tl208",
    9: "Po212",
    10: "Pb208",
    11: "e+",
    12: "At211",
    13: "Po211",
    14: "Bi207",
    15: "Pb207",
    16: "Ra226",
    17: "Rn222",
    18: "Po218",
    19: "Pb214",
    20: "Bi214",
    21: "Po214",
    22: "Pb210",
    23: "Bi210",
    24: "Po210",
    25: "Pb206",
    26: "Tl210",
    27: "Tl206",
    28: "Hg206",
    29: "At218",
    30: "Rn218",
    31: "Co60",
    32: "Ra223",
    33: "Rn219",
    34: "Po215",
    35: "Pb211",
    36: "Bi211",
    37: "Tl207",
    38: "Ra225",
    39: "Rn221",
    40: "Po217",
    41: "Pb213",
    42: "Ac225",
    43: "Fr221",
    44: "Ra221",
    45: "Rn217",
    46: "Po213",
    47: "Pb209",
    48: "Bi209",
    49: "Tl205",
    50: "At217",
    51: "Bi213",
    52: "Tl209",
}


def load_origin_id_map() -> Dict[int, str]:
    """Build origin_id -> label from AlphaGlueRn222/include/SteppingAction.hh particleOriginMap."""
    script_dir = Path(__file__).resolve().parent
    header = script_dir / "AlphaGlueRn222" / "include" / "SteppingAction.hh"
    if not header.exists():
        return {}

    text = header.read_text(encoding="utf-8", errors="ignore")
    m = re.search(
        r"std::map<G4String,\s*G4int>\s+particleOriginMap\{(.*?)\n\s*\};",
        text,
        flags=re.S,
    )
    if not m:
        return {}

    block = m.group(1)
    pairs = re.findall(r'\{"([^"]+)",\s*(-?\d+)\}', block)

    out: Dict[int, str] = {}
    for name, code_str in pairs:
        code = int(code_str)
        if code not in out:
            out[code] = name
    return out


ORIGIN_ID_MAP: Dict[int, str] = load_origin_id_map()


def load_reverse_origin_map() -> Dict[int, str]:
    """Build reverse_origin_map (origin_id -> parent isotope) from SteppingAction.hh."""
    script_dir = Path(__file__).resolve().parent
    header = script_dir / "AlphaGlueRn222" / "include" / "SteppingAction.hh"
    if not header.exists():
        return {}

    text = header.read_text(encoding="utf-8", errors="ignore")
    m = re.search(
        r"std::map<G4int,\s*G4String>\s+reverseParticleOriginMap\{(.*?)\n\s*\};",
        text,
        flags=re.S,
    )
    if not m:
        return {}

    block = m.group(1)
    pairs = re.findall(r"\{(-?\d+),\s*\"([^\"]+)\"\}", block)
    out: Dict[int, str] = {}
    for code_str, name in pairs:
        out[int(code_str)] = name
    return out


# Exported maps for analysis notebooks:
# - ORIGIN_ID_MAP: full origin code map (e.g. 73 -> gammaPo214, 62 -> e-Bi214)
# - REVERSE_ORIGIN_MAP: parent-isotope-only map (e.g. 44 -> Rn222)
REVERSE_ORIGIN_MAP: Dict[int, str] = load_reverse_origin_map()


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Read AlphaGlueRn222 phase-space .bin files (legacy 16-double and current 18-double formats)."
    )
    parser.add_argument(
        "inputs",
        nargs="+",
        help="Input file(s) or glob pattern(s), e.g. 'generated_jobs/2mm_load_50um_top_1k/*.bin'",
    )
    parser.add_argument(
        "--record-len",
        choices=["auto", "16", "18"],
        default="auto",
        help="Number of doubles per record. Default: auto.",
    )
    parser.add_argument(
        "--head",
        type=int,
        default=5,
        help="Number of rows to print per file (0 to suppress). Default: 5.",
    )
    parser.add_argument(
        "--summary",
        action="store_true",
        help="Print summary statistics per file.",
    )
    parser.add_argument(
        "--summary-max-rows",
        type=int,
        default=25,
        help="Max rows to print in grouped summary tables (particle_id/copy_no). Use -1 for all. Default: 25.",
    )
    parser.add_argument(
        "--csv-out",
        type=str,
        default="",
        help="Write parsed rows to CSV. With multiple files, use --combine to write one merged CSV.",
    )
    parser.add_argument(
        "--combine",
        action="store_true",
        help="Combine all files into one table (adds source_file column).",
    )
    return parser.parse_args()


def resolve_inputs(patterns: Sequence[str]) -> List[str]:
    files: List[str] = []
    for pattern in patterns:
        matches = sorted(glob.glob(pattern, recursive=True))
        if matches:
            files.extend(matches)
        elif os.path.isfile(pattern):
            files.append(pattern)
    return sorted(set(files))


def detect_record_len(file_size: int, forced: str) -> int:
    if forced != "auto":
        return int(forced)

    d16 = 16 * 8
    d18 = 18 * 8
    ok16 = (file_size % d16) == 0
    ok18 = (file_size % d18) == 0

    if ok18 and not ok16:
        return 18
    if ok16 and not ok18:
        return 16
    if ok16 and ok18:
        return 18

    raise ValueError(
        f"File size {file_size} is not divisible by 16*8 or 18*8 bytes; this does not look like an AlphaGlueRn222 PS bin."
    )


def read_file(path: str, forced_record_len: str) -> Tuple[np.ndarray, List[str], int]:
    file_size = os.path.getsize(path)
    record_len = detect_record_len(file_size, forced_record_len)
    cols = EXTENDED_COLUMNS if record_len == 18 else BASE_COLUMNS

    raw = np.fromfile(path, dtype="<f8")
    if raw.size % record_len != 0:
        raise ValueError(
            f"{path}: raw double count ({raw.size}) is not divisible by record length ({record_len})."
        )

    data = raw.reshape((-1, record_len))
    return data, cols, record_len


def write_csv(path: str, data: np.ndarray, columns: Sequence[str]) -> None:
    with open(path, "w", newline="", encoding="utf-8") as f:
        writer = csv.writer(f)
        writer.writerow(columns)
        writer.writerows(data.tolist())


def print_head(data: np.ndarray, columns: Sequence[str], n: int) -> None:
    if n <= 0:
        return
    n = min(n, len(data))
    if n == 0:
        print("  (no rows)")
        return
    print("  " + ", ".join(columns))
    for row in data[:n]:
        formatted = []
        for val in row:
            if abs(val - round(val)) < 1e-12 and abs(val) < 1e9:
                formatted.append(str(int(round(val))))
            else:
                formatted.append(f"{val:.8g}")
        print("  " + ", ".join(formatted))


def id_label(pid: int) -> str:
    name = PARTICLE_ID_MAP.get(pid, "unknown")
    return f"{pid} ({name})"


def origin_label(origin_id: int) -> str:
    name = ORIGIN_ID_MAP.get(origin_id, "unknown")
    return f"{origin_id} ({name})"


def _print_group_counts(
    values: np.ndarray,
    label: str,
    format_key,
    max_rows: int,
) -> None:
    unique, counts = np.unique(values, return_counts=True)
    order = np.argsort(counts)[::-1]
    unique = unique[order]
    counts = counts[order]

    n_total = len(unique)
    n_show = n_total if max_rows < 0 else min(max_rows, n_total)
    print(f"  {label} counts (showing {n_show}/{n_total}):")
    for key, cnt in zip(unique[:n_show], counts[:n_show]):
        print(f"    {format_key(int(key))}: {int(cnt)}")
    if n_show < n_total:
        hidden = int(np.sum(counts[n_show:]))
        print(f"    ... ({n_total - n_show} more groups, {hidden} rows)")


def print_summary(data: np.ndarray, columns: Sequence[str], max_rows: int) -> None:
    col_idx = {name: i for i, name in enumerate(columns)}
    n_rows = data.shape[0]
    print(f"  rows: {n_rows}")
    if n_rows == 0:
        return

    if "event_id" in col_idx:
        event_ids = data[:, col_idx["event_id"]].astype(np.int64)
        print(f"  unique event_id: {len(np.unique(event_ids))}")
        print(f"  event_id range: [{event_ids.min()}, {event_ids.max()}]")

    if "particle_id" in col_idx:
        pids_f = data[:, col_idx["particle_id"]]
        pids = pids_f.astype(np.int64)
        _print_group_counts(pids, "particle_id", id_label, max_rows)
        near_int = np.isclose(pids_f, np.rint(pids_f), atol=1e-8)
        known = np.array([pid in PARTICLE_ID_MAP for pid in pids], dtype=bool)
        if near_int.mean() < 0.95 or known.mean() < 0.5:
            print(
                "  warning: particle_id values look unusual; file may not be a decay PS bin (or may use a different format)."
            )

    if "copy_no" in col_idx:
        copies = data[:, col_idx["copy_no"]].astype(np.int64)
        print(f"  unique copy_no: {len(np.unique(copies))}")
        _print_group_counts(copies, "copy_no", str, max_rows)

    if "origin_id" in col_idx:
        origins = data[:, col_idx["origin_id"]].astype(np.int64)
        print(f"  unique origin_id: {len(np.unique(origins))}")
        _print_group_counts(origins, "origin_id", origin_label, max_rows)

    if "energy_MeV" in col_idx:
        en = data[:, col_idx["energy_MeV"]]
        print(f"  energy_MeV range: [{en.min():.6g}, {en.max():.6g}]")

    if "time_s" in col_idx:
        t = data[:, col_idx["time_s"]]
        print(f"  time_s range: [{t.min():.6g}, {t.max():.6g}]")


def main() -> int:
    args = parse_args()
    files = resolve_inputs(args.inputs)
    if not files:
        print("No files matched the provided inputs.", file=sys.stderr)
        return 2

    if args.csv_out and len(files) > 1 and not args.combine:
        print(
            "--csv-out with multiple files requires --combine (or run once per file).",
            file=sys.stderr,
        )
        return 2

    if args.combine:
        merged_arrays = []
        merged_cols: List[str] | None = None
        for path in files:
            data, cols, record_len = read_file(path, args.record_len)
            if merged_cols is None:
                merged_cols = ["source_file"] + cols
            row_src = np.array([[path]] * len(data), dtype=object)
            merged = np.empty((len(data), len(cols) + 1), dtype=object)
            merged[:, 0] = row_src[:, 0]
            merged[:, 1:] = data
            merged_arrays.append(merged)
            print(f"{path}")
            print(f"  detected record_len={record_len} doubles ({record_len * 8} bytes/record)")
            if args.summary:
                print_summary(data, cols, args.summary_max_rows)
            if args.head > 0:
                print_head(data, cols, args.head)

        merged_out = np.vstack(merged_arrays) if merged_arrays else np.empty((0, 0))
        if args.csv_out:
            with open(args.csv_out, "w", newline="", encoding="utf-8") as f:
                writer = csv.writer(f)
                writer.writerow(merged_cols or [])
                if merged_out.size:
                    writer.writerows(merged_out.tolist())
            print(f"Wrote merged CSV: {args.csv_out}")
        return 0

    for path in files:
        data, cols, record_len = read_file(path, args.record_len)
        print(f"{path}")
        print(f"  detected record_len={record_len} doubles ({record_len * 8} bytes/record)")
        if args.summary:
            print_summary(data, cols, args.summary_max_rows)
        if args.head > 0:
            print_head(data, cols, args.head)

        if args.csv_out:
            write_csv(args.csv_out, data, cols)
            print(f"Wrote CSV: {args.csv_out}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
