# Signal-inference metrics for theranostic source monitoring

**Scope.** This document specifies a small set of metrics built from the
detected gamma signal that the photon-detector panels record around the water
phantom. Each metric targets one of three clinically motivated questions:

1. *How much of the loaded glue activity has diffused into the surrounding
   water?* (**M1 – diffusion**)
2. *How much radioactive material is present in the measured area?*
   (**M2 – material amount**)
3. *Is there a compact secondary source somewhere outside the applicator, and
   if so, where?* (**M3 – secondary source and its location**)

Companion implementation: `Simulation_output_signal_inference_metrics.ipynb`.
That notebook already contains a working first pass of all three metrics; this
document is the design reference behind it, so the notebook can be re-derived,
audited, and extended without re-reading the cells. Existing repeated-run
infrastructure from
`Detector_performance_analysis_Rn222_repetition_comparison.ipynb` and
`Detector_performance_analysis_Rn222_activity_normalized.ipynb` is reused for
loading, panel binning, and baseline subtraction.

---

## 1. Setup and notation

### 1.1 Geometry summary

The simulation geometry, taken from the modelling note (`theranostics_modelling_results.tex`):

- Water phantom: 20 cm × 20 cm × 150 cm, centred at the origin. The long axis
  is `z`, so `z ∈ [−750, +750]` mm.
- Glue applicator: 40 mm × 2 mm × 40 mm, centred at `(0, 101, 0)` mm — on the
  `+y` face of the water phantom.
- Six silicon detector panels at 20 cm from the water surface on each face
  (`±x`, `±y`, `±z`).
- Two source configurations matter for M3: a "diffuse" source filling the
  water volume, and a "localised" 5 cm cube centred at `(0, 0, +500)` mm.

### 1.2 Cases used for calibration

| Case        | Glue fraction | Secondary source              | Notes                          |
|-------------|--------------:|-------------------------------|--------------------------------|
| `baseline`  | 1.00          | none                          | calibration anchor             |
| `diff10`    | 0.90          | 10 % diffuse in water volume  | small diffusion                |
| `diff50`    | 0.50          | 50 % diffuse in water volume  | mid diffusion                  |
| `diff90`    | 0.10          | 90 % diffuse in water volume  | large diffusion                |
| `loc20`     | 0.80          | 20 % in 5 cm cube at `z=+500` | localised secondary, low-frac. |
| `loc40`     | 0.60          | 40 % in 5 cm cube at `z=+500` | localised secondary, mid-frac. |
| `loc60`     | 0.40          | 60 % in 5 cm cube at `z=+500` | localised secondary, high-frac.|

Each case has ten repetitions × 10 000 primary Rn-222 events. Across
repetitions the standard deviation of detector counts is small (≲ 1 %), which
is what supports treating M1–M3 as point estimators with a per-repetition
uncertainty.

### 1.3 Observables

A single acquisition produces a table of *detector entries* — gamma photons
crossing the inward face of one of the six panels. We restrict to
`particleName == 'gamma'` and `boundary == 'enter'`. For each entry we keep:

```
volumeName, x_mm, y_mm, z_mm, time_s, kineticEnergy_MeV
```

Let `D = {NegX, PosX, NegY, PosY, NegZ, PosZ}`. For an analysis window
`t ≤ T`:

- `N_p`  = number of detector entries on panel `p`.
- `h_p(z)` = histogram of `z_mm` of entries on panel `p`, bin centres `z_k`,
  bin width `Δz` (default 60 bins on `[−1500, +1500]` mm — matching the
  existing notebooks).
- `r_p = N_p / N_primaries` = panel rate per simulated primary decay.
  For real data this becomes `N_p / T_live` (counts per unit live time) and
  is converted to physical activity via a calibration source.
- `\tilde h_p(z) = h_p(z) / N_p` = shape-normalised z-profile.

We use two compound views built from sums of panels:

- `Ysum = +y ∪ −y` — longitudinal profile from the panels closest to / opposite
  the applicator.
- `Xsum = +x ∪ −x` — longitudinal profile from the side panels (good imaging
  of deep, off-applicator activity).

### 1.4 Baseline reference

Let `\bar r_p^B` and `\bar{\tilde h}_p^B(z)` be the per-panel rate and shape
averaged over the ten `baseline` repetitions. These are the anchors for the
*residual* observables used by M1 and M3:

```
Δh_p(z)  =  \tilde h_p(z)  −  \bar{\tilde h}_p^B(z)        (shape residual)
δr_p     =  r_p          −  \bar r_p^B                     (rate residual)
```

The repetition-level standard deviation `σ_B(p, z)` gives the bin-wise noise
floor used in z-score and Mahalanobis-style statistics below.

---

## 2. M1 — Diffusion metrics

### 2.1 Physical intuition

When part of the activity migrates from the glue layer into the water volume,
three things happen to the detected signal:

1. The forward (`+y`) panel response drops, because the applicator no longer
   contains all the activity and a fraction of it now sits below the
   applicator (away from `+y`).
2. The longitudinal `z`-profile broadens, because activity is distributed
   throughout the 150 cm phantom rather than concentrated at the applicator.
3. The side (`±x`) and `−y` panels gain a larger share of the total count.

Total count is *not* a good diffusion observable: from the panel-rate table,
total detector entries drop only from 12 500 (baseline) to 12 057 (diff90) —
a ≲ 4 % change. Profile shape and panel balance change by tens of percent.

### 2.2 Intrinsic metrics

**M1.a — Forward-panel containment.**

```
F_{+y}  =  N_{+y} / (N_{+y} + N_{−y} + N_{+x} + N_{−x})
A_{y}   =  (N_{+y} − N_{−y}) / (N_{+y} + N_{−y})
```

`F_{+y}` is the fraction of side-panel hits arriving on `+y`. `A_y` is the
y-panel asymmetry. Both decrease monotonically with diffusion: from the
panel-rate table

| Case      | N_{+y} | N_{−y} | A_y  | F_{+y} |
|-----------|-------:|-------:|-----:|-------:|
| baseline  |  5882  | 1420   | 0.61 | 0.47   |
| diff10    |  5552  | 1550   | 0.56 | 0.45   |
| diff50    |  4412  | 2143   | 0.35 | 0.36   |
| diff90    |  3178  | 2736   | 0.07 | 0.27   |

Both quantities are robust because they are *ratios* of detected counts and
are essentially independent of the absolute activity.

**M1.b — Longitudinal profile width.**

For the `Ysum` view, define the count-weighted variance of `z`:

```
μ_z      = Σ_k z_k · h^{Ysum}(z_k)  /  Σ_k h^{Ysum}(z_k)
σ_z²     = Σ_k (z_k − μ_z)² · h^{Ysum}(z_k)  /  Σ_k h^{Ysum}(z_k)
W^{Ysum} = z_{Q90} − z_{Q10}                  (inter-quantile width, robust)
```

`σ_z` and `W^{Ysum}` increase monotonically with diffusion. From the existing
`material_signal_amount_summary.csv`:

| Case      | z\_q80 width [mm] |
|-----------|------------------:|
| baseline  | 575               |
| diff10    | 639               |
| diff50    | 911               |
| diff90    | 1116              |
| loc20     | 763               |
| loc40     | 815               |
| loc60     | 802               |

The width also responds to `loc*` cases, but the panel-asymmetry vector (see
M3) cleanly separates them from genuine diffusion.

**M1.c — Central-z forward fraction.**

```
C_{+y}  =  N_{+y}( |z| ≤ 150 mm )  /  N_{+y}
```

`C_{+y}` decreases as activity drifts longitudinally away from the applicator
footprint. It is sensitive to *both* diffusion and longitudinal displacement,
so it is only used as a diffusion indicator after M3 has ruled out a localised
secondary.

### 2.3 Diffusion-fraction estimators

Each of M1.a–c monotonically encodes diffusion, but no single one of them is
calibrated to a percent value. We adopt **two estimators side-by-side**:

**E1 — Closed-form monotone fit (intrinsic).** A single-variable monotone
calibration of true diffusion fraction against one of the intrinsic metrics.
We use `A_y` as the primary observable because it spans the largest dynamic
range across the diffuse cases (0.61 → 0.07, factor of ≈ 9):

```
f̂_{diff}^{E1}  =  g( A_y )
```

with `g` an interpolating curve fit to the four calibration points
`{(A_y^B, 0%), (A_y^{diff10}, 10%), (A_y^{diff50}, 50%), (A_y^{diff90}, 90%)}`.
The notebook uses a piecewise monotone PCHIP interpolation (or a 3-rd-order
polynomial in `A_y`) so the fit is fully closed-form and reads off three
curves: one for `A_y`, one for `σ_z^{Ysum}`, one for `C_{+y}`. Each of the
three single-variable fits is reported as a sanity check on the others.

**E2 — Ridge regression on shape features (operational).** A multivariate
regression that uses the full feature stack already constructed in the
notebook:

```
Features: shape-normalised z-profile bins (Ysum + Xsum + All),
          σ_z and W^{Ysum},
          C_{+y} (= "*_central_fraction"),
          A_y, A_x = (N_{+x} − N_{−x}) / (N_{+x} + N_{−x}),
          positive-residual L1 and centroid summaries
Model:    StandardScaler ∘ RidgeCV ( α ∈ 1e-4 .. 1e4 )
Training: cases ∈ {baseline, diff10, diff50, diff90} only
          (loc20, loc40, loc60 are excluded — they are a different hypothesis)
Validation: leave-one-repetition-out
```

Current LOO performance from `diffusion_fraction_cv_summary.csv`: MAE = 0.6
percentage points, R² = 0.999.

Both estimators are reported in the final table. Where they disagree the
single-variable closed-form fit is preferred for interpretability and the
Ridge fit for accuracy.

**Reporting rule.** Quote either form of `\hat f_{\mathrm{diff}}` *only* for
candidates that M3 classifies as "no compact secondary source". When M3
raises a secondary flag, the diffusion estimator is undefined (it would
absorb the localised signal into a fake diffusion).

### 2.4 Limitations

- Calibration is at 0 / 10 / 50 / 90 %. Intermediate values rely on the smooth
  Ridge interpolation; a 25 % or 75 % simulation campaign would close that
  gap.
- The estimator assumes the applicator is positioned as in the geometry.
  Translations or rotations of the applicator would require retraining.

---

## 3. M2 — Material/signal-amount metrics

### 3.1 What "amount" means

There are two distinct questions one might want to answer:

- **Q-total:** how much activity is present *anywhere* in the imaged volume?
- **Q-applicator:** how much activity is *still on the applicator*?

The metrics below split cleanly along that line.

### 3.2 Q-total — total detected rate

The simplest activity proxy is the total detector-entry rate per primary
decay:

```
R_{\mathrm{tot}}  =  ( Σ_{p ∈ D} N_p )  /  N_{\mathrm{primaries}}
```

For real measurements, replace `N_primaries` with live time and apply a
calibration constant `k_{\mathrm{cal}}` (counts per Bq, established from a
reference source):

```
\hat A  =  R_{\mathrm{tot}} / k_{\mathrm{cal}}            (absolute activity)
\hat A  =  A_{\mathrm{ref}} · R_{\mathrm{tot}} / R_{\mathrm{tot}}^B   (baseline-relative)
```

The second form is what the existing notebook calls
`baseline_equivalent_activity_uCi`, anchored at `A_{\mathrm{ref}} = 3 µCi` per
the DaRT reference. From `material_signal_amount_summary.csv`,
`R_{\mathrm{tot}}/R^B_{\mathrm{tot}}` stays within 1 ± 0.013 for every
configuration — confirming that total rate is geometry-insensitive and is the
right observable for total material amount.

### 3.3 Q-applicator — central-ROI forward signal

To estimate the fraction of activity still located on the applicator, restrict
to the volume that the applicator illuminates strongly: the `+y` panel within
the central z-ROI. The **canonical** applicator-retained metric is therefore

```
R_{\mathrm{app}}  =  N_{+y}( |z| ≤ 150 mm, t ≤ T )  /  N_{\mathrm{primaries}}
\hat f_{\mathrm{app}}  =  R_{\mathrm{app}} / R^B_{\mathrm{app}}
```

The `+y`-only definition is preferred over the previous All-panel
`relative_central_signal_vs_baseline` because it explicitly weights the
applicator hemisphere: it gives the cleanest "how much of the activity is
still where it was put" interpretation. The All-panel version is kept as a
secondary, cross-checking observable since it is what the previous abstract
already reports.

`\hat f_{\mathrm{app}}` is a "fraction of activity still on the applicator"
proxy *iff* no secondary source is present. With a secondary source the
central-ROI signal is suppressed both by displacement and by the secondary
sitting outside the central ROI, so the two effects mix. The reporting rule is
the same as for M1: quote `\hat f_{\mathrm{app}}` only after M3 has cleared a
secondary-source hypothesis; otherwise report it jointly with the secondary
contribution from M3.

### 3.4 Time-window scaling

All M2 quantities are reported for a fixed analysis window `t ≤ T`. The
existing convention is `T = 10⁶ s` (about 11 days), which captures the early
Rn-222 progeny chain (Po-214 + Bi-214) while suppressing the late
Bi-210-dominated tail. Shorter cumulative windows (2 h, 12 h, 1 d, 2 d, 5 d)
are available from the activity-normalised analysis and should be reported
alongside as growth curves for *the same* M2 quantities.

For real data, the activity-calibration constant `k_{\mathrm{cal}}` must
itself be time-window-matched, because the photon yield per Rn-222 decay
depends on which fraction of the decay chain has accumulated.

### 3.5 Limitations

- Detector efficiency, dead time, and energy thresholds are not yet modelled.
  M2 absolute values are therefore "ideal detector" estimates; only the
  relative-to-baseline ratios are robust.
- Background subtraction is not yet included; in real data, `R_{\mathrm{tot}}`
  must be background-corrected before computing `\hat A`.

---

## 4. M3 — Secondary-source metrics

### 4.1 Physical signature

A compact off-applicator source has two characteristic effects:

1. A *positive residual* in `\Δh_p(z) = \tilde h_p(z) − \bar{\tilde h}^B_p(z)`
   concentrated near the projection of the source onto the panel.
2. A *panel-balance shift*: an off-axis source preferentially illuminates the
   panel(s) facing it, breaking the baseline ±-symmetry along the
   corresponding axis. For the simulated configuration with the cube at
   `z = +500` mm, the strongest signature is the `PosZ` panel rate
   exploding while the `NegZ` panel stays near baseline.

These are qualitatively distinct from M1's signature: diffusion broadens the
profile *symmetrically*; a localised secondary produces an *asymmetric* bump.

### 4.2 M3.a — Panel-asymmetry vector (detection)

For each axis define

```
A_x  =  (N_{+x} − N_{−x}) / (N_{+x} + N_{−x})
A_y  =  (N_{+y} − N_{−y}) / (N_{+y} + N_{−y})
A_z  =  (N_{+z} − N_{−z}) / (N_{+z} + N_{−z})
```

From the panel-rate table:

| Case      | A_x        | A_y    | A_z       | notes                                                  |
|-----------|-----------:|-------:|----------:|--------------------------------------------------------|
| baseline  |  +0.005    | +0.618 | −0.063    | reference                                              |
| diff10    |  +0.002    | +0.571 |  +0.070   | A_y reduced by diffusion; A_z noise floor              |
| diff50    |  +0.001    | +0.352 | −0.026    | A_y strongly reduced; A_z near baseline                |
| diff90    |   0.000    | +0.076 | −0.001    | A_y near zero; A_z near baseline                       |
| loc40     |  −0.002    | +0.402 |  +0.857   | **δA_z very large**; A_y also reduced                  |
| loc60     |  +0.001    | +0.282 |  +0.917   | **δA_z very large**; A_y reduced                       |

`A_y` moves dramatically under both diffusion *and* a localised secondary
source, so a χ² built on `(A_x, A_y, A_z)` will fire indiscriminately on
both. To make M3.a a *specific* test for a localised off-axis secondary,
we restrict the asymmetry vector to the components that are symmetric under
diffusion but break under off-axis displacement: `(A_x, A_z)`. Define

```
δA  =  ( A_x − A_x^B,  A_z − A_z^B )
```

and the **Mahalanobis-style detection statistic** using the baseline
repetition covariance `Σ_B` of `(A_x, A_z)`:

```
χ²_{sec}  =  δA^T  Σ_B^{−1}  δA          (2 degrees of freedom)
```

`Σ_B` is estimated from the ten baseline repetitions. Threshold choice for
χ²₂:

| Threshold | P(χ²₂ > t)  | σ-equivalent |
|----------:|------------:|-------------:|
| 10.0      | 6.7 × 10⁻³  | ≈ 2.7 σ      |
| 13.8      | 1.0 × 10⁻³  | ≈ 3.3 σ      |
| 25.0      | 3.7 × 10⁻⁶  | ≈ 4.9 σ      |

Empirically on the current campaign, baseline and all `diff*` cases give low
secondary-source probabilities in the logistic cross-check, while `loc20`,
`loc40`, and `loc60` are all classified as compact secondary-source cases.
The notebook uses a default threshold of `χ²_{sec} > 10`
which sits comfortably between the two regimes and has expected false-
positive rate ≪ 1 per the 60 simulated repetitions. Users with cleaner
baseline statistics (more reps, more events per rep) can move the threshold
up to 13.8 or 25 for the corresponding tighter false-positive rate without
losing sensitivity to the simulated `loc*` cases.

By construction, the localized cases (`loc20`/`loc40`/`loc60`) give
`‖δA‖ ≫ baseline scatter` because their `A_z` shifts strongly.

For a secondary at `(0, 0, 0)` (on-axis with the applicator but inside the
phantom rather than on the surface), all three asymmetries would still be
near baseline; that case is resolved instead by the *profile shape*
indicators in M3.b–c. M3.a is therefore the specific test for an off-axis
displacement; M3.b–c are the catch-all.

### 4.3 M3.b — Positive-residual L1 score (model-free detection)

The asymmetry vector loses information about within-panel structure. To
recover that, compute the positive residual integrated over a search ROI:

```
S_{sec}^{p}    =  Σ_{(z) ∈ ROI}  max( Δh_p(z), 0 )
S_{sec}^{tot}  =  Σ_p  S_{sec}^{p}
```

Default ROI for the present geometry: `z ∈ [+250, +750] mm` on the `Ysum` and
`Xsum` views. Generalised to 2-D on the `±x` panels:
`(y, z) ∈ [−100, +100] × [+250, +750] mm`. Normalise by the baseline
repetition spread:

```
z-score:  Z_{sec}  =  ( S_{sec}^{tot}  −  S_{B}^{tot} )  /  σ( S_{B}^{tot} )
```

This is the bin-by-bin version of M3.a and gives the same conclusion: large
positive `Z_{sec}` for `loc*`, near-zero for diff-only cases.

### 4.4 M3.c — Location estimator (positive-residual centroid)

Given a positive-secondary detection, estimate the secondary location from
the centroid of the positive residual on the panel that images it best. For
the present geometry, the `Xsum` view (= `+x` and `−x` panels) is the best
projector of a deep source onto a single z-axis:

```
ẑ_{sec}  =  Σ_z  z · max( Δh^{Xsum}(z), 0 )  /  Σ_z  max( Δh^{Xsum}(z), 0 )
```

A 2-D version on a single `±x` panel produces `(ŷ, ẑ)` jointly:

```
(ŷ, ẑ)_{sec}  =  centroid of  max(Δh^{Xsum}(y, z), 0)  over (y, z) ∈ ROI
```

For triangulation in 3-D, combine the `Xsum` centroid (sees `y, z`) with the
`Ysum` centroid (sees `x, z`):

```
ẑ_{sec}  =  weighted mean of  ẑ^{Xsum}  and  ẑ^{Ysum}
ŷ_{sec}  =  ŷ^{Xsum}
x̂_{sec}  =  x̂^{Ysum}
```

Current performance from `secondary_source_summary.csv` (true z = +500 mm):

| Case      | ẑ^{Xsum} [mm] | ẑ^{Ysum} [mm] | All-panel peak z [mm] |
|-----------|---------------:|---------------:|----------------------:|
| loc20     | see regenerated outputs | see regenerated outputs | see regenerated outputs |
| loc40     | 525 ± 5        | 531 ± 5        | 515 ± 21              |
| loc60     | 529 ± 3        | 534 ± 3        | 530 ± 28              |

Recovered z is biased high by ≈ 25–34 mm. The bias is consistent with the
cube being a 50 mm volume (so its `+z` face sits at +525 mm), with a small
extra forward bias from the centroid being pulled by the panel-acceptance
envelope. Calibrating that bias out requires more secondary-location
configurations (multiple `z`, plus a few off-axis x/y).

### 4.5 M3.d — Combined classifier (cross-check)

A logistic regression on the residual L1 features, the panel-asymmetry
vector, the residual centroid, and the secondary-ROI rate (standardised
features, `class_weight='balanced'`) is kept as a cross-check on M3.a.
Leave-one-repetition-out gives ROC AUC = 1.0 on the present six cases.

**Primary detector: M3.a (Mahalanobis χ² on the panel-asymmetry vector).**
The closed-form statistic is preferred because it is fully interpretable, has
no fitting step beyond estimating the baseline covariance `Σ_B`, and already
achieves ‖δA‖/σ_B much larger than five on both `loc*` cases. M3.d is a
sanity check that the richer feature set agrees.

### 4.6 Limitations

- Only one secondary-source location is available (`z = +500` mm). Location
  estimation is therefore validated on one point; a more complete location
  calibration requires at least 3–4 additional cube positions (e.g. mid-z,
  on-axis vs. off-axis y).
- A secondary source placed at `(0, 0, 0)` would produce no z-panel asymmetry
  and a flat y-balance change consistent with diffusion — i.e. it is
  degenerate with diffusion under the current observables alone. Resolving
  that case needs the profile-shape information (a localised central source
  still produces a compact `Ysum` peak, while diffusion produces a broad
  one).

---

## 5. Cross-case validation table

Expected qualitative behaviour of every metric across the six cases.
`↑` / `↓` mark monotonic increase / decrease relative to `baseline`; `≈` means
within ±5 % of baseline; `★` marks the dominant signature.

| Metric                                    | baseline | diff10 | diff50 | diff90 | loc40 | loc60 |
|-------------------------------------------|:--------:|:------:|:------:|:------:|:-----:|:-----:|
| **M1 forward containment** `F_{+y}`       | high     | ↓ a bit| ↓      | ↓↓     | ↓     | ↓↓    |
| **M1 y-asymmetry** `A_y`                  | high     | ↓ a bit| ↓      | ↓↓ ★   | ↓     | ↓     |
| **M1 z-width** `σ_z^{Ysum}`               | small    | ↑      | ↑      | ↑↑ ★   | ↑     | ↑     |
| **M1 central forward** `C_{+y}`           | high     | ↓ a bit| ↓      | ↓↓     | ↓     | ↓↓    |
| **M2 total rate** `R_{tot}`               | ≈        | ≈      | ≈      | ≈      | ≈     | ≈     |
| **M2 applicator fraction** `\hat f_{app}` | 1.00     | ≈ 0.94 | ≈ 0.72 | ≈ 0.47 | 0.67  | 0.50  |
| **M3 z-asymmetry** `\|A_z − A_z^B\|`     | 0        | ≈ 0    | ≈ 0    | ≈ 0    | ↑↑ ★  | ↑↑↑ ★ |
| **M3 χ²\_sec**                            | ≈ 0      | ≈ 0    | small  | small  | ↑↑↑ ★ | ↑↑↑ ★ |
| **M3 hotspot ẑ**                          | undefined| undef  | undef  | undef  | ≈ 525 | ≈ 530 |

The reading order is:
1. Compute M3 first to decide whether a secondary source is present.
2. If M3 is positive: report the secondary location, then re-run M2 outside
   the secondary region for a clean applicator-amount estimate.
3. If M3 is negative: M1 produces a diffusion fraction and M2 produces total +
   applicator-retained amounts directly.

---

## 6. Implementation plan for the notebook

A **fresh notebook** —
`Signal_inference_metrics.ipynb` — is created alongside the existing
`Simulation_output_signal_inference_metrics.ipynb`. The existing notebook is
kept as a working reference; the new notebook is structured to map cell-by-cell
onto sections §2–§5 of this document. Outline:

0. **Imports + configuration.** Reuse the case map, panel sets, z-binning, and
   manifest loader from the existing notebook.
1. **Feature extraction.** Per-repetition feature table with: panel counts /
   rates, asymmetries `(A_x, A_y, A_z)`, `F_{+y}`, profile moments
   (`σ_z`, quantile widths), central-ROI fractions, and positive-residual
   summaries against the baseline mean profiles.
2. **M1 — diffusion.**
   - **E1 (closed-form).** Plot the calibration of `A_y`, `σ_z^{Ysum}`, and
     `C_{+y}` against true diffusion fraction across `{baseline, diff10,
     diff50, diff90}`. Fit a monotone PCHIP (or low-order polynomial) for
     each. Report `f̂_{diff}^{E1}` from each of the three observables.
   - **E2 (Ridge).** Re-run the multivariate Ridge + LOO already in the
     existing notebook, report MAE / R² / per-case mean ± sd.
   - Side-by-side comparison plot of E1 vs E2.
3. **M2 — amount.**
   - Compute `R_{tot}` and `R_{tot}/R^B_{tot}` per repetition; convert to
     `\hat A` at the 3 µCi reference.
   - Compute `R_{app}^{+y}` (canonical) and the All-panel cross-check.
   - Time-window growth: report `R_{tot}` and `\hat f_{app}` at
     `T ∈ {2 h, 12 h, 1 d, 2 d, 5 d, 10⁶ s}`.
4. **M3 — secondary source.**
   - **M3.a (primary).** Closed-form Mahalanobis χ² on `(A_x, A_y, A_z)` with
     `Σ_B` from baseline repetitions. Threshold and detection table.
   - **M3.b.** Positive-residual L1 score in the search ROI; z-score vs
     baseline noise floor.
   - **M3.c.** Positive-residual centroid `ẑ_{sec}` from `Xsum` and `Ysum`,
     plus combined estimate. Placeholder for `(x̂, ŷ)` when more cube
     locations are simulated.
   - **M3.d (cross-check).** Logistic regression with LOO ROC AUC; agreement
     check vs. M3.a.
5. **Cross-case validation.** Reproduce the qualitative table in §5
   programmatically from the feature table — this is the regression-test of
   the notebook against this document.
6. **Reporter.** For each repetition, print the inference cascade:
   ```
   case  →  M3 verdict (yes/no, χ²_sec, ẑ if yes)
                 ↓
            if no  : M1 diffusion fraction (E1 & E2), M2 total amount,
                     applicator-retained amount
            if yes : secondary location, M2 total amount, applicator amount
   ```
   Save the report as a CSV in
   `outputs/Signal_inference_metrics/inference_report.csv` (new directory,
   parallel to the existing one).

---

## 7. Open questions and follow-ups

- **Secondary location coverage.** Adding cube positions at e.g.
  `(0, 0, −500)`, `(0, 0, 0)`, `(0, 50, 0)`, `(±50, 0, 0)` would let M3.c be
  calibrated for bias and uncertainty across the phantom, and would close the
  diffusion vs. centred-secondary degeneracy noted in §4.6.
- **Detector response.** Energy resolution, efficiency, dead time, and
  background are not in the simulation. Each of these reshapes the relative
  weights inside M2 and M3 but does not change the metric definitions.
- **Activity-window matching.** When `k_{\mathrm{cal}}` is established, it
  should be derived for the *same* `t ≤ T` cut used in M2, because the photon
  yield per Rn-222 decay depends on which fraction of the chain has decayed.
- **Diffusion estimator outside the calibration range.** A 25 % and 75 %
  diffusion simulation would tighten interpolation; right now `f_{diff}` is
  defined on the convex hull of 0/10/50/90.

---

## 8. File map

- `Signal_inference_metrics.ipynb` — implementation aligned with this
  document (new, fresh).
- `outputs/Signal_inference_metrics/` — produced CSVs (new directory; same
  table schema as the existing one plus `inference_report.csv`).
- `docs/figures/Signal_inference_metrics/` — produced PNGs.
- `Simulation_output_signal_inference_metrics.ipynb` — earlier working
  reference; kept as a first pass.
- `docs/signal_inference_metrics.md` — this document.
- `docs/theranostics_modelling_results.tex` — base modelling note.
- `docs/theranostics_abstract_with_figures.tex` — conference abstract.
