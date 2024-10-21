# ANTS3 mission

* The toolkit was created to greatly facilitate particle and optical photon simulations for the students
   * Interactive graphical interface and scrpting interface (Python and Javascript) for Geant4
 * Quick (and not so dirty) simulations during initial stahges of the development of position-sensitive detectors
 * Many features which are missing in "vanilla" Geant4 are added to make life simpler

# ANTS3 highlights

* Monte Carlo particle simulations: an adavanced frontend for Geant4 toolkit
   * Multi-process simulation
   * Interactive GUI for configuration and visualisation of the results
   * Custom scoring infrastructure
   * Flexible options for generation of primary particles
   * Direct Interface from energy deposition to optical photon generation
 * Custom optical tracer based on CERN ROOT 3D navigation
   * Fresnel-based tracing combined with
   * Custom rules for optical interfaces
   * Possibility to add "functional objects" in photon tracing, allowing to easlily connect Monte Carlo tracing and formula-driven transport (e.g. thin lense, optical fiber)
   * Flexible scoring and history recording
 * Detector configuration, simulations and processing of results (integrated CERN ROOT) are possible  to conduct either in GUI or using scripts
   * Python interface
   * JavaScript interface
   * 
# Status of the development

* Particle simulation part is ready  
* Optical simulation part is in late beta
* GUI is mostly ready
* Scripting system is ready, slight tweaking is ongoing
* Computer farm support is in early alpha
* Infrastructure for help system for scripting method is ready, help text is in the process of filling
* Warning: ANTS3 documentation is not ready!

---

# Installation instructions
* [Ubuntu 24.04 Mate](https://github.com/andrmor/ANTS3bundle/wiki/Install_Ubuntu24.04)
* [Outdated: Ubuntu 22.04 Mate](https://github.com/andrmor/ANTS3bundle/wiki/Install_Ubuntu22.04)
 
 ---

 # Latest tested software versions

* 21 Octpber 2024 --> Ubuntu 24.04 Mate, Qt 6.8.0, CERN ROOT 6.32.04, Geant4 11.2.2
* 27 June 2024 --> Ubuntu 24.04 Mate, Qt 6.6.3, CERN ROOT 6.32.02, Geant4 11.2.2

