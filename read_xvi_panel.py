#!/usr/bin/env python3
"""Read one Bristol XVI panel image without combining it with other panels."""

from __future__ import annotations

import argparse
import json
from pathlib import Path

import numpy as np


def read_panel(path: Path) -> tuple[np.ndarray, dict]:
    metadata_path = path.with_suffix(".json")
    metadata = json.loads(metadata_path.read_text(encoding="utf-8"))
    expected = metadata["rows"] * metadata["columns"]
    image = np.fromfile(path, dtype=np.float32)
    if image.size != expected:
        raise ValueError(
            f"{path} contains {image.size} pixels; metadata expects {expected}"
        )
    return image.reshape(metadata["rows"], metadata["columns"]), metadata


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Inspect one independent XVI energy-deposition panel image."
    )
    parser.add_argument("image", type=Path, help="Path to one *_xvi_*_edep.raw file")
    parser.add_argument(
        "--npy-out", type=Path, help="Optional path for the same panel as a NumPy array"
    )
    args = parser.parse_args()

    image, metadata = read_panel(args.image)
    nonzero = int(np.count_nonzero(image))
    print(f"panel: {metadata['panel_name']} (id {metadata['panel_id']})")
    print(f"shape: {image.shape}; pitch: {metadata['pixel_pitch_mm']} mm")
    print(f"nonzero pixels: {nonzero}")
    print(f"image sum: {image.sum(dtype=np.float64):.12g} {metadata['unit']}")

    if args.npy_out:
        np.save(args.npy_out, image)


if __name__ == "__main__":
    main()
