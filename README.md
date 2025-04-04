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

* Particle and optical simulation parts are ready  
* GUI is ready
* Scripting system is ready
* Computer farm support is ready (to be refactored later)
* Infrastructure for help system for scripting method is ready, help text is in the process of filling
* Warning: work on ANTS3 documentation has not yet started!
* Working on the paper desribing ANTS3

---

# Installation instructions
* [Ubuntu 24.04 Mate](https://github.com/andrmor/ANTS3bundle/wiki/Install_Ubuntu24.04_Qt6)
* [Outdated: Ubuntu 24.04 Mate with Qt5](https://github.com/andrmor/ANTS3bundle/wiki/Install_Ubuntu22.04_Qt5)
* [Outdated: Ubuntu 22.04 Mate](https://github.com/andrmor/ANTS3bundle/wiki/Install_Ubuntu22.04)
 
 ---

# Farm mode instructions
* On every farm node computer start dispatcher executable from a terminal, suppling three parameters: IP address, port, maxNumberOfProcesses
  * For example (replace the 'x'es by the actual IP!):
    
   /ants3bundle/build/Desktop_Qt_6_8_2-Debug/bin/dispatcher xxx.xxx.xxx.xxx 12344 6
* At the main window of ANTS3 click "Workload"
* Check "Use farm" box
* Click "New" and fill the IP and port for every farm node
* The simulations in ANTS3 will be automatically distributed over the farm nodes
 ---

 # Latest tested software versions

* 22 February 2025 --> Ubuntu 24.04 Mate, Qt 6.8.2, CERN ROOT 6.32.10, Geant4 11.3.0
* 21 October 2024 --> Ubuntu 24.04 Mate, Qt 6.8.0, CERN ROOT 6.32.04, Geant4 11.2.2
* 27 June 2024 --> Ubuntu 24.04 Mate, Qt 6.6.3, CERN ROOT 6.32.02, Geant4 11.2.2

