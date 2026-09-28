# Bristol XVI readout model

The simulation places six independent flat panels at the existing `+X`, `-X`,
`+Y`, `-Y`, `+Z`, and `-Z` detector locations. Each panel is a
`409.6 mm x 409.6 mm` CsI slab sampled as `1024 x 1024` pixels at `0.4 mm`
physical pitch. The local `+Z` axis of every panel points away from the phantom.

The CsI thickness is provisionally `1.0 mm`, matching the total thickness of the
previous ideal silicon plate. This value must be replaced when a detector-specific
measurement or manufacturer specification becomes available. It can be varied
before `/run/initialize` with:

```text
/det/set_xviScintillatorThickness 1 mm
```

At the end of a run, each panel is written independently as:

```text
<output-stem>_xvi_pos_x_edep.raw
<output-stem>_xvi_pos_x_edep.json
...
<output-stem>_xvi_neg_z_edep.raw
<output-stem>_xvi_neg_z_edep.json
```

The raw image contains `1024 x 1024` native-endian `float32` values in row-major
`[y, x]` order. Values are weighted energy deposit in MeV. The matching JSON file
records the panel identity, dimensions, pitch, units, hit-step count, and total
energy deposit. Keeping one file per panel reflects a single clinical readout and
prevents implicit summation of opposing views.

The executable currently fixes the Geant4 worker count to one. Keep this setting
for XVI output; per-worker image reduction has not yet been implemented.

Read one image with:

```bash
python3 read_xvi_panel.py run_xvi_pos_x_edep.raw
```

The current model scores energy deposition only. Scintillator light spread,
pixel gain, pedestal, common-mode noise, electronic noise, saturation, and bad
pixel correction should be applied as later response stages.
