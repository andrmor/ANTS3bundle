# ANTS3 source navigation for AI

**Purpose:** Locate implementation code when the ANTS3 AI Knowledge Base does not resolve an ANTS3-specific question. This is a **navigation map**, not an API reference or a source of configuration-generation rules.

**Location and paths:** This document belongs at the **repository root**, alongside `README.md` and the `ants3bundle/` directory. **Every path below is relative to that repository root**, not to the `script/`, `geo/`, or `src/ants3/` directory. The main application source is under `ants3bundle/src/ants3/`. The map reflects the `dev` branch; confirm paths and behavior against the revision actually being investigated.

**How to use it:** Open the indicated `.cpp` file **and its matching `.h` file** in the same directory. For a scripting command, first identify the registered scripting-unit name and C++ interface class; then follow the interface implementation into the underlying subsystem. Do not assume an internal method is available to scripts, or that source-level support implies a working runtime feature.

## Scripting registration and execution

**First stop for a scripting-unit name:** `ants3bundle/src/ants3/script/ascripthub.cpp`, constructor `AScriptHub::AScriptHub()`; check `ants3bundle/src/ants3/script/ascripthub.h` as well. The constructor registers the principal scripting units. For example, JavaScript `hist` maps to `AHist_SI`, `graph` to `AGraph_SI`, `tracks` to `ATrackRec_SI`, and `partan` to `APartAnalysis_SI`. **Scripting names are not necessarily C++ class names.** Some GUI-only units are registered separately during GUI initialization.

| Investigation | Begin with (repository-root-relative paths) |
|---|---|
| Scripting-unit registration, hub coordination and configuration synchronization | `ants3bundle/src/ants3/script/ascripthub.cpp` (`AScriptHub`); see the adjacent header. |
| Shared script-manager behavior and registered-unit name protection | `ants3bundle/src/ants3/script/avirtualscriptmanager.cpp`. |
| JavaScript evaluation, worker thread and QObject exposure | `ants3bundle/src/ants3/script/ajscriptmanager.cpp` and `ants3bundle/src/ants3/script/ajscriptworker.cpp`; inspect `AJScriptWorker::onRegisterInterface()` for `QJSEngine` registration. |
| Python manager, worker and QVariant/Python conversion (when enabled) | `ants3bundle/src/ants3/script/Python/apythonscriptmanager.cpp`, `ants3bundle/src/ants3/script/Python/apythonworker.cpp`, `ants3bundle/src/ants3/script/Python/apythoninterface.cpp`. |
| GUI-only graph-window scripting unit `grwin` | `ants3bundle/src/ants3/script/ScriptInterfaces/agraphwin_si.cpp` (`AGraphWin_SI`); trace its GUI registration separately from `graph` (`AGraph_SI`). |

## Main subsystem entry points

Find the relevant **script interface, configuration record and implementation** rather than assuming any one file establishes the entire behavior. Open each indicated implementation together with its matching header.

| Need to locate | Begin with (repository-root-relative paths) |
|---|---|
| Configuration load/save and scripting config updates | `ants3bundle/src/ants3/script/ScriptInterfaces/aconfig_si.cpp`; follow the configuration holder and its JSON readers/writers. |
| Geometry tree, serialization, placement, arrays and prototypes | `ants3bundle/src/ants3/geo/ageoobject.cpp`, `ants3bundle/src/ants3/geo/ageometryhub.cpp`. |
| Shape classes and JSON parameters | `ants3bundle/src/ants3/geo/ageoshape.cpp`; follow the specific shape class. |
| Geometry scripting and GeoConsts | `ants3bundle/src/ants3/script/ScriptInterfaces/ageo_si.cpp`; follow geometry-constant definitions into `geo/`. |
| Materials and chemical composition | See **Materials and composition** below; their definitions are primarily under `ants3bundle/src/ants3/materials/`. |
| Geometry roles, monitors and calorimeters | Search `ants3bundle/src/ants3/geo/` for the relevant special-role record; follow its JSON reader and scorer registration. |
| Particle sources, energies, spatial/angular distributions | `ants3bundle/src/ants3/particleSim/aparticlesourcerecord.cpp`; follow the selected generator and settings. |
| File-based particle input and output | Search `ants3bundle/src/ants3/particleSim/` for file-particle generators, their settings and particle export; confirm formats in the readers/writers. |
| Particle run setup and output collection | `ants3bundle/src/ants3/particleSim/aparticlesimmanager.cpp`; follow settings and output merging. |
| Monitor, calorimeter and analyzer scripting results | `ants3bundle/src/ants3/script/ScriptInterfaces/aparticlesim_si.cpp`; follow scoring containers, including `ants3bundle/src/ants3/particleSim/aparticleanalyzerhub.cpp`. |
| Tracking-history file format and low-level traversal (`tracks`) | `ants3bundle/src/ants3/particleSim/atrackingdataimporter.cpp`, `ants3bundle/src/ants3/script/ScriptInterfaces/atrackrec_si.cpp`. |
| High-level tracking analysis (`partan`) | `ants3bundle/src/ants3/script/ScriptInterfaces/apartanalysis_si.cpp`; follow the crawler and analysis processors. |
| Histogram and graph scripting units (`hist`, `graph`) | `ants3bundle/src/ants3/script/ScriptInterfaces/ahist_si.cpp` (`AHist_SI`) and `ants3bundle/src/ants3/script/ScriptInterfaces/agraph_si.cpp` (`AGraph_SI`); helper types are under `ants3bundle/src/ants3/tools/` (e.g. `ahistogram`, `ath`). |
| Graph window, basket, overlays and GUI scripting (`grwin`) | `ants3bundle/src/ants3/script/ScriptInterfaces/agraphwin_si.cpp` (`AGraphWin_SI`); follow `AGraphWindow`, basket managers and invoked GUI operations. |
| Text output, numerical files and script errors | `ants3bundle/src/ants3/script/ScriptInterfaces/acore_si.cpp`; follow scripting-engine registration for JavaScript/Python differences. |

## Materials and composition

| Investigation | Begin with (repository-root-relative paths) |
|---|---|
| Material collection, names/indices, JSON persistence and ROOT media | `ants3bundle/src/ants3/materials/amaterialhub.cpp` (`AMaterialHub`). |
| Individual material definitions and properties, including optical/scintillation properties | `ants3bundle/src/ants3/materials/amaterial.cpp` (`AMaterial`). |
| Chemical composition strings, isotope specifications, nested mixtures, mass/molar fractions, gas pressure and density | `ants3bundle/src/ants3/materials/amatcomposition.cpp` (`AMatComposition`). |
| Material assignment to geometry | `ants3bundle/src/ants3/geo/ageoobject.cpp`; follow geometry construction in `ants3bundle/src/ants3/geo/ageometryhub.cpp`. |

Treat **bulk optical properties** separately from **interface rules**. For Geant4 materials, NCrystal and other backend-specific behavior, follow the downstream consumer in particle transport or external integration. Material serialization alone does not establish runtime support.

## Photon bombs: flood generation

| Investigation | Begin with (repository-root-relative paths) |
|---|---|
| Flood settings, JSON keys, conversion of old `rectangle`/`ring` configs | `ants3bundle/src/ants3/photonSim/aphotonsimsettings.cpp` (`AFloodSettings`, `AFloodProfile`). |
| Position sampling (profiles, grids, formulas, validation) shared by lsim and the GUI | `ants3bundle/src/ants3/photonSim/afloodruntime.cpp` (`AFloodRuntime`, `AFloodCoordSampler`). |
| Use in the photon simulator | `ants3bundle/src/lsim/aphotonsimulator.cpp` (`setupPhotonBombs`, `simulateFlood`). |
| GUI: Flood page, test-generation dialog, outline drawing | `ants3bundle/src/ants3/gui/photsim/aphotsimwin.cpp` (`setupFloodGui`, `updateFloodGui`), `afloodtestdialog.cpp`, `aphotonsourceplotter.cpp`. |

## Areas not yet mapped in detail

ANTS3 also has a **custom optical-photon tracer**, sensor responses, light-response models and position reconstruction; these are not Geant4 optical-photon simulation features. Until their locations are mapped for the relevant revision, search the repository for `PhotonSim`, `Photon`, `Sensor`, `InterfaceRules`, `LightResponse`, `Mercury` and `reconstruction`, then follow actual declarations and call sites. For Geant4 physics-list registration, optional NCrystal support, datasets and packaging, search the **entire repository**, not just the application source directory. Do not infer build availability from a source file alone.

## Verification path

1. **Identify the revision.** Use the requested branch/commit; do not claim to know HEAD without verifying it. If browsing fails, try the direct raw-file path.
2. **Find the public boundary.** For configurations, inspect the JSON reader/writer and consumer; for scripting, find registration and the scripting interface. Read the complete relevant `.h` declaration and `.cpp` implementation.
3. **Follow actual behavior.** A serialized field may be ignored by a consumer; an internal C++/ROOT/Geant4 method may not be exposed to ANTS3 scripts. Check optional build flags and external data when relevant.
4. **Describe the evidence precisely.** Source-verified is not the same as runtime-tested. If essential source/build details are inaccessible or inconclusive, state the limitation rather than inventing behavior.

This map is intentionally selective. Search for new topics rather than guessing an undocumented path or treating this table as exhaustive.
