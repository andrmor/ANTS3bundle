# DavisLUT surface model — implementation record

Adds a look-up-table (LUT) based optical interface rule implementing the approach of
E. Roncali & S. R. Cherry, *Phys. Med. Biol.* **58** (2013) 2185 ("simulation of light
transport based on 3D characterization of crystal surfaces"), plus an offline generator
that builds the LUTs by ray tracing over a measured surface topography (e.g. AFM), plus
GUI and scripting integration.

Rule type string: **`DavisLUT`**, abbreviation **`LUT`**.

Unlike the existing `RoughSurface` rule (a single sampled microfacet), the LUT contains
the full angular reflectance/transmittance of the surface precomputed with multiple
micro-reflections, shadowing and masking, so it stays accurate at large incidence angles.

---

## New files

### `src/ants3/photonSim/interfaceRules/alutsurfacedata.{h,cpp}` — class `ALutSurfaceData`
Container + (de)serialization + runtime sampling of one surface LUT.
- Schema: metadata (`n1`, `n2`, wavelength, source heightmap, grid, generation stats) +
  per-incidence-angle integer counts (`Launched/ReflectedCounts/TransmittedCounts/AbsorbedCounts`)
  + flattened 2D `(thetaOut x phiOut)` histograms for the reflected and transmitted rays.
  `FormatVersion` gates the reader so a wavelength axis can be added later.
- `writeToJson()` / `readFromJson()` — JSON I/O (custom `.lut` format).
- `check()` — validates binning, array sizes and per-bin count conservation
  (`Refl+Trans+Abs == Launched`).
- `buildRuntime()` — precomputes per-bin reflection/transmission probabilities and the
  inverse-CDF sampling tables (called once before simulation; read-only afterwards).
- `selectThetaBin(thetaDeg, rnd)` — incidence-angle bin, stochastically interpolated
  between adjacent bin centers.
- `sampleOutgoing(reflected, iBin, r1,r2,r3, thetaOut, phiOut)` — samples an outgoing
  direction from the LUT distribution (inverse CDF + within-bin smoothing).

### `src/ants3/photonSim/interfaceRules/alutinterfacerule.{h,cpp}` — class `ALutInterfaceRule : AInterfaceRule`
The runtime interface rule.
- `calculate(photon, normal)` — computes the incidence angle vs the global normal, picks
  the angle bin, decides reflect/transmit/absorb from the LUT, samples `(thetaOut,phiOut)`,
  and rebuilds the outgoing direction in the incidence-plane frame. Returns `Back`/`Forward`.
- `loadLUT(fileName)` — reads a `.lut` file into `Data`.
- `getMaterialConsistencyWarning()` — non-blocking check that the LUT `n1/n2` match the
  refractive indices of the assigned materials.
- `initializeWaveResolved()` / `doCheckOverrideData()` — prepare the runtime CDFs.
- `doWriteToJson()` / `doReadFromJson()` — embeds the full LUT in the config JSON (required
  so it reaches the `lsim` worker processes).
- `canHaveRoughSurface()` = false (the LUT *is* the surface model; surface stays `Polished`).
- `canBeSymmetric()` = false (the LUT bakes in the `n1 -> n2` direction).
- The output direction is explicitly renormalized before returning — see "Bug found" below.

### `src/ants3/photonSim/interfaceRules/alutsurfacegenerator.{h,cpp}` — class `ALutSurfaceGenerator`
Offline generator (built into `ants3` only, not `lsim`).
- `loadHeightmapMatrix(file, dx, dy)` — ASCII matrix of heights (rows<->y, cols<->x).
- `loadHeightmapXYZ(file)` — 3-column x/y/z on a regular grid.
- `generate(result)` — ray traces `photonsPerThetaBin` photons per incidence-angle bin over
  the tiled surface; fills an `ALutSurfaceData`. Config fields: `n1,n2,wavelength,
  thetaIncBins/thetaOutBins/phiOutBins,photonsPerThetaBin,phiSteps,maxBounces,seed,
  reverseGeometry`, plus a `progressCallback` for abort/progress.
- Internals: grid->triangles; 2D DDA traversal (`findIntersection`) with Möller-Trumbore
  (`intersectTriangle`); per-facet unpolarized Fresnel (`fresnelReflection`, incl. TIR) with
  specular reflection or Snell refraction about the *local* normal; multi-bounce loop
  (`tracePhoton`); periodic lateral tiling. Reports mean bounces, anomalies and wrap count.

### `src/ants3/script/ScriptInterfaces/ainterfacerules_si.{h,cpp}` — class `AInterfaceRules_SI`
Script unit registered as **`rules`**.
- `generateSurfaceLut(heightmapFile, outLutFile, params)` — runs the generator (params:
  `n1,n2,wavelength,pixelSizeX/Y,format,photonsPerBin,thetaBins,thetaOutBins,phiOutBins,
  phiSteps,maxBounces,seed,comment,reverseGeometry,alsoReverse`); returns generation stats.
- `getLutInfo(lutFile)` — returns a LUT's metadata/binning.
- `setLutMaterialRule(matFrom, matTo, lutFile)` / `setLutVolumeRule(volFrom, volTo, lutFile)`
  — create a `DavisLUT` rule from a `.lut` file and assign it.
- `clearMaterialRule(...)` / `clearVolumeRule(...)`.

---

## Modified files

- `photonSim/interfaceRules/ainterfacerule.h` — added `virtual bool canBeSymmetric()`
  (default true; false for direction-specific rules such as DavisLUT).
- `photonSim/interfaceRules/ainterfacerule.cpp` — registered `"DavisLUT"` in
  `interfaceRuleFactory()` and `getAllInterfaceRuleTypes()`.
- `photonSim/interfaceRules/ainterfacerulehub.h` — added `announceRulesChanged()` (emits the
  existing `rulesLoaded` signal so the GUI refreshes after script-side edits).
- `photonSim/interfaceRules/ainterfacerulehub.cpp` — **bug fix** in `checkAll()`: the volume-
  rule loop reported per-rule errors only when a material-rule error already existed
  (`if(!err..)` -> `if(!es..)`). Pre-existing bug affecting all rule types, exposed while
  wiring DavisLUT as a volume rule.
- `gui/photsim/ainterfacewidgetfactory.{h,cpp}` — new editor widget `ALutInterfaceWidget`
  (Load-LUT, R/T-vs-angle plot, per-bin outgoing distribution, n1/n2-mismatch warning) plus
  its `dynamic_cast` branch in `createEditWidget()`.
- `gui/photsim/ainterfaceruledialog.cpp` — disable/uncheck the "Symmetric" box when the rule
  reports `!canBeSymmetric()`.
- `script/ascripthub.cpp` — register the new `rules` script unit.
- `src/ants3/ants3.pro`, `src/lsim/lsim.pro` — build entries. `ants3.pro` also gained a
  fallback for `python3-config` when the default is too old for `--embed` (was pointing at a
  3.6 build and breaking the link on this machine).

No changes were needed in the photon tracer, the interface-rule tester, or the dispatcher/
farm path.

---

## Workflow: from an AFM surface to a DOI simulation

End-to-end recipe (this is exactly the pipeline used for the 5/14/28 um LYSO validation;
helper scripts live in `ants3bundle/script/LUT_test_results/`).

**1. Preprocess the AFM scan into a heightmap the generator can read.**
A raw AFM export (`*.spm.txt`) is a flat, multi-channel column list, not a height grid, so it
must be converted first. Extract the height column (col 1 = trace, col 4 = retrace), reshape to
the scan grid (e.g. 512x512, row-major), convert to a single length unit, and subtract a best-fit
plane (levels out sample tilt). Write it as either:
- a **matrix** file (rows = y, columns = x, one height per cell) -> use `format:"matrix"` and
  give `pixelSizeX/Y`; pixel size = scan_size / (N-1) in the same units as the heights, or
- an **x y z** three-column file on a regular grid -> use `format:"xyz"`.
Helper: `prep_afm.py` (does exactly this; heights written in um, pixel = 10 um / 511).

**2. Generate the LUT** from the leveled heightmap, in the ANTS3 Script window:
```js
rules.generateSurfaceLut("lyso_leveled.txt", "lyso.lut",
    { n1:1.824, n2:1.0, wavelength:420,
      pixelSizeX:0.0195694, pixelSizeY:0.0195694,   // um (10um / 511)
      format:"matrix", photonsPerBin:40000, alsoReverse:true });
```
`n1` = index of the medium the photons come from (crystal), `n2` = medium behind the surface
(air/grease). `alsoReverse:true` additionally writes `lyso_reverse.lut` with n1/n2 swapped, for
the return direction (air->crystal). The call returns generation stats (mean bounces, etc.).
Inspect any file later with `rules.getLutInfo("lyso.lut")`.

*Ensemble over several scans (optional but recommended).* The script unit generates one LUT from
one heightmap; to average several locations of the same crystal, call `generateSurfaceLut` once
per location, then **sum their integer count arrays** (`Launched/Reflected/Transmitted/
AbsorbedCounts` and the two histograms) into one pooled `.lut` — equivalent to ray-tracing over
all patches together. This pooling is a small pure-Python post-step: `pool_luts.py`
(`python3 pool_luts.py pooled.lut loc1.lut loc2.lut ...`). It could be folded into
`generateSurfaceLut` later.

**3. Assign the rule** to the crystal<->outside interface, either from a script:
```js
rules.setLutMaterialRule("LYSO", "air", "lyso.lut");          // crystal -> air
rules.setLutMaterialRule("air", "LYSO", "lyso_reverse.lut");  // air -> crystal (reverse LUT)
```
(or `setLutVolumeRule` for a volume-name pair), **or** from the GUI (next section). The LUT data
are embedded into the config, so the `.lut` file is not needed at run time.

**4. Run the photon simulation** as usual — `lsim.simulate()` from the Script window, or the
headless worker `lsim <workdir> <config.json> <id>` (when running the worker directly, set
`PhotonSim.Run.EventFrom=0`, `EventTo=<Flood.Number>`, and a non-zero `Seed`, which the GUI
otherwise fills in per worker).

**5. Analyze** `SensorSignals.txt` (per-event sensor signals). For the dual-ended DOI studies
here the observable is `(s0-s1)/(s0+s1)` vs source depth; `plot_signal_vs_depth_hist_fits.py`
histograms it per depth with Gaussian fits.

---

## Using the DavisLUT rule in the GUI

**Generating a LUT** is script-only (no GUI dialog yet): open the **Script** window and call
`rules.generateSurfaceLut(...)` as above. Everything else can be done in the GUI.

**Assigning / editing the rule:**
1. Open the **interface-rule window** (photon-simulation GUI). It shows a material x material
   matrix and a volume-pair list.
2. **Double-click the cell** for the interface you want (e.g. row = LYSO, column = air).
   The interface-rule dialog opens.
3. In the **rule-type combo box**, choose **`DavisLUT`**. The DavisLUT editor panel appears.
4. Click **Load LUT** and pick the `.lut` file. The info line shows `n1 -> n2`, wavelength,
   binning and mean bounces. A **red warning** appears if the LUT's `n1/n2` disagree (>1%) with
   the refractive indices of the two materials this cell connects — a guard against assigning a
   LUT to the wrong material pair or direction.
5. Inspect the LUT: **R/T vs angle** plots the reflection and transmission probability against
   incidence angle; enter an incidence angle, pick **Reflected/Transmitted**, and
   **Show angular distribution** to view the 2D (theta_out, phi_out) outgoing map for that angle.
6. The **Symmetric** checkbox is **disabled** for DavisLUT — the LUT bakes in the n1->n2
   direction, so assign the reverse-direction LUT to the opposite cell (air -> LYSO) separately,
   using a LUT generated with swapped indices (`alsoReverse:true`).
7. **Accept**. The rule (with its embedded LUT) is now in the configuration; save the config as
   usual. The rule also appears in the material matrix with the abbreviation **`LUT`**.

**Checking it before a run:** the interface-rule **tester** (in the same GUI) shoots test photons
at the selected rule and draws the resulting reflected/transmitted directions — a quick way to
confirm the LUT behaves as expected before launching a full simulation.

**Running:** build the geometry and run the photon simulation as normal; the DavisLUT rule is
applied automatically at the assigned interface, in both the GUI and headless/farm runs.

---

## Bug found and fixed while testing

`ALutInterfaceRule::calculate()` returns the photon direction directly (the tracer takes it
as-is for non-rough rules, without renormalizing). Even though the direction is unit by
construction, a floating-point `|v| = 1+epsilon` made a downstream
`acos(sensorNormal . v)` in `ASensorModel::getAngularFactor()` return NaN at a near-normal
sensor hit, which then indexed the angular-response array out of bounds and crashed `lsim`.
Fix: explicitly renormalize the output direction at the end of `calculate()`. (The old
`RoughSurface` rule avoided this because its Fresnel path in the tracer renormalizes.)

---

## What was tested

Verification used small standalone harnesses linked against the compiled object files, plus a
full end-to-end run in `lsim`.

1. **Flat-plane analytic check** — a zero heightmap, n1=1.824, n2=1.0:
   - generated `R(theta)` matches the analytic unpolarized Fresnel curve to within 0.003,
     with the total-internal-reflection step at the 33.3 deg critical angle;
   - reflected rays land at `thetaOut = thetaInc`, transmitted at the Snell angle, both in the
     incidence plane (`phiRel ~ 0`) — confirms the generator<->rule frame convention;
   - per-bin count conservation holds; `buildRuntime/selectThetaBin/sampleOutgoing` round-trip.

2. **Rough (egg-carton) surface** — multi-bounce (>1 avg), strongly broadened angular
   distributions, count conservation, and JSON write/read round-trip all pass. Anomaly rate
   scales with surface steepness (grazing/multi-bounce rays that cannot be cleanly classified
   are excluded from the LUT and reported in its metadata).

3. **End-to-end on real data** — `script/LYSO-28um-...-loc1.spm.txt` (AFM, plane-leveled) ->
   forward+reverse LUTs -> config with LYSO<->air rules swapped to DavisLUT -> `lsim` depth
   scan vs the old `RoughSurface` rule. The LUT method produced a clean, monotonic DOI
   response with a stronger depth slope and lower/more depth-dependent light collection,
   consistent with the paper. Artifacts and a comparison plot are in
   `ants3bundle/script/LUT_test_results/` (see its README for the pipeline and caveats).

Both `ants3` and `lsim` build cleanly (qmake, Qt 6.5.3, ROOT 6.28/04 on this machine).

---

## Generator escape handling (investigation — the most sensitive modeling choice)

Early rough-surface runs discarded a few-to-16 percent of photons as "anomalies" (higher for
steeper surfaces). Investigation with per-cause counters showed:

- It was **not** near-horizontal DDA-cap exhaustion and **not** the bounce limit (both zero).
- A **watertight ray/triangle intersection** (Woop-Benthin-Wald) was implemented and tested to
  rule out grazing edge-leaks; it produced byte-identical results (zero effect), so that
  hypothesis was rejected and the watertight code was reverted (the generator keeps the
  simpler, validated Moller-Trumbore `intersectTriangle`).
- **Cause:** a height-field multi-bounce situation. A photon reflecting off a steep facet inside
  a micro-valley can fly upward and escape over a ridge without re-hitting the surface, so its
  flight direction (upward) disagrees with its medium (still the incident crystal, since it only
  ever reflected). ~13% of photons on these steep surfaces, concentrated at grazing incidence.

Three ways to classify such an escaping photon were implemented and compared:

| handling | grazing R(80 deg) | photons | DOI vs old microfacet |
|---|---|---|---|
| discard (initial) | ~0.97 (on kept) | loses ~13% | much stronger (artefact) |
| geometry / direction | ~0.84 | conserved | much stronger (artefact) |
| **medium-based (Roncali & Cherry)** | **~0.97** | **conserved** | **≈ identical** |

**Committed default: medium-based**, following the paper: a photon's fate is set by the medium
it is in (last event a reflection => still in the crystal => reflected; has transmitted through
=> in the outer medium => transmitted), never discarded. It is byte-identical on the flat-plane
case (TIR still R=1 above the critical angle), reproduces the paper's fig-7a reflectance shape,
and conserves photons. Diagnostic getters `upEscapeReclassified()` / `downEscapeReclassified()` /
`degenerateDiscarded()` report the minority whose direction disagrees with their medium.

**Key finding:** the escape handling is the single most sensitive modeling choice here — far more
than watertight-vs-MT (zero effect). An earlier draft of these notes claimed the LUT gives
"stronger depth dependence than the microfacet model"; that was an **artefact of the discard /
geometry handling**. With the paper-faithful medium-based rule the ensemble LUT and the old
`CustomNormal` microfacet model (which is itself built from the *same* measured AFM normals) give
**nearly identical** DOI-vs-depth for this crystal (e.g. -0.202 vs -0.197 at 13 mm). This does
not contradict Roncali & Cherry (whose comparison was against the cruder single-sigma UNIFIED
Gaussian, not a measured-normal distribution) — it is two measured-surface models agreeing, a
mutual-consistency check. See `ants3bundle/script/LUT_test_results/`:
`pooled_Rtheta_3way.png`, `pooled_DOI_old_vs_LUT.png`, and `*_discard.* / *_geom.*` for the
alternative-handling artifacts.

---

## Known limitations (v1)

- Single-wavelength LUT (applied to all photons regardless of `waveIndex`); a wavelength axis
  is the intended `FormatVersion 2` extension.
- LUT generation is single-threaded and exposed via script only (no GUI "generate" dialog yet).
- The periodic-tiling seam (heightmap edges do not match) remains a minor unhandled
  discontinuity; mirror-tiling the map would remove it if needed.
