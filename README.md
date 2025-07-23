# ANTS3 highlights

* Monte Carlo particle simulations: an adavanced frontend for Geant4 toolkit
   * Multi-process simulation with computer farm support
   * Interactive GUI for configuration and visualisation of the results
   * Custom "convinience" objects in geometry definition: stacks, arrays, protoype/instances etc
   * Custom scoring infrastructure (monitors, calorimeters, particle analyzers etc)
   * Flexible options for generation of primary particles (sources, files, multiple-stage simulations)
   * Direct interface from energy deposition to optical photon generation
 * Custom optical tracer based on CERN ROOT 3D navigation module
   * Fresnel-based photon tracing 
   * Custom rules for optical interfaces, rough surfaces, interface tester infrastructure
   * Possibility to add "functional objects": complement Monte Carlo tracing with matematical expression-driven transport (e.g. thin lense, optical fiber)
   * Flexible scoring and tracing history recording
 * Scripting support with full access to configuration, simulation and processing of the results
   * Python interface
   * JavaScript interface
 * Unsupervised optimization of the detector parameters based on scripting system involving Simplex minimizer
 * WebSocket server for creating custom extensions
     
# Status of the development

* Full code released
* ANTS3 paper is submitted, the preprint: https://arxiv.org/abs/2506.07922

# Next steps

* Popularting the ANTS3 wiki
* Adding more examples (config and scripts)
* Filling material and source libraries

---

# Installation instructions
* [Ubuntu 24.04 Mate](https://github.com/andrmor/ANTS3bundle/wiki/Install_Ubuntu24.04_Qt6)
* [Outdated: Ubuntu 24.04 Mate with Qt5](https://github.com/andrmor/ANTS3bundle/wiki/Install_Ubuntu22.04_Qt5)
* [Outdated: Ubuntu 22.04 Mate](https://github.com/andrmor/ANTS3bundle/wiki/Install_Ubuntu22.04)«

# How to use Farm mode
* On every farm node computer start dispatcher executable from a terminal, suppling three parameters: IP address, port, maxNumberOfProcesses
  * For example (replace the 'x'es by the actual IP!):
    
   /ants3bundle/build/Desktop_Qt_6_8_2-Debug/bin/dispatcher xxx.xxx.xxx.xxx 12344 6
* At the main window of ANTS3 click "Workload"
* Check "Use farm" box
* Click "New" and fill the IP and port for every farm node
* The simulations in ANTS3 will be automatically distributed over the farm nodes

 ---

 # Latest tested software versions

* 21 April 2025 --> Ubuntu 24.04 Mate, Qt 6.9.0, CERN ROOT 6.32.10, Geant4 11.3.0
* 22 February 2025 --> Ubuntu 24.04 Mate, Qt 6.8.2, CERN ROOT 6.32.10, Geant4 11.3.0
* 21 October 2024 --> Ubuntu 24.04 Mate, Qt 6.8.0, CERN ROOT 6.32.04, Geant4 11.2.2
* 27 June 2024 --> Ubuntu 24.04 Mate, Qt 6.6.3, CERN ROOT 6.32.02, Geant4 11.2.2

