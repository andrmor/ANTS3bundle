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
 * Position reconstruction of events using statistical techniques based on the light response model of the detector
 * WebSocket server for creating custom extensions
     
# A paper describing ANTS3

* Published in Compter Physics Communications: https://doi.org/10.1016/j.cpc.2025.109869
  * Preprint: https://arxiv.org/abs/2506.07922

# New features in version 2.0 (released on August 3, 2026)
* Added support for [Mercury](https://github.com/vovasolo/pymercury) position reconstruction library (previously integrated in Ants2)
   * Two new scripting units: "response" (light response model handling) and "mercury" (position reconstruction)
* Added photon simulation mode based on light response model
* The concept of intrinsic energy resolution for primary scintillation is replaced with Fano factor formalism
* Relative gains of the light sensors can be configured directly (see "Sensors" window)
* Geometry constants can be used in particle and photon sources
* Add simulation of annihilation gamma acolinearity 
* Refactor photon simulation GUI
* Refactor indication of particle/photon sources (consolidated controls are at the geometry window)

# New features in version 1.07 (released on September 29, 2025)
* [EcoMug](https://github.com/dr4kan/EcoMug) soure of cosmic muons can be used in particle simulations
* [NCrystal](https://github.com/mctools/ncrystal) support: see installation instructions below ("Optional features")
  
---

# Installation instructions
* [Ubuntu 24.04 Mate](https://github.com/andrmor/ANTS3bundle/wiki/Install_Ubuntu24.04_Qt6)
* [Outdated: Ubuntu 24.04 Mate with Qt5](https://github.com/andrmor/ANTS3bundle/wiki/Install_Ubuntu22.04_Qt5)
* [Outdated: Ubuntu 22.04 Mate](https://github.com/andrmor/ANTS3bundle/wiki/Install_Ubuntu22.04)

# Optional features, requiring separate installation
* [NCrystal](https://github.com/mctools/ncrystal) library for neutron scattering: see installation [instructions](https://github.com/andrmor/ANTS3bundle/wiki/NCrystalInstall)

# Test on a virtual machine
* It is possible to try Ants3 v1.07 using a pre-configured virtual machine: follow installation [instructions](https://github.com/andrmor/ANTS3bundle/wiki/VirtualMachine)
* The development team is working to provide a Flatpak installation, expected at the end of August 2026

# How to use Farm mode
* On every farm node computer start dispatcher executable from a terminal, supplying three parameters: IP address, port, maxNumberOfProcesses
  * For example (replace the 'x'es by the actual IP!):
    
   /ants3bundle/build/Desktop_Qt_6_8_2-Debug/bin/dispatcher xxx.xxx.xxx.xxx 12344 6
* At the main window of ANTS3 click "Workload"
* Check "Use farm" box
* Click "New" and fill the IP and port for every farm node
* The simulations in ANTS3 will be automatically distributed over the farm nodes

 ---

 # Latest tested software versions

* 3 August 2026 --> Ubuntu Mate 24.04, Qt 6.11.0, CERN ROOT 6.32.10, Geant4 11.3.2
* 19 September 2025 --> Ubuntu Mate 24.04, Qt 6.9.2, CERN ROOT 6.32.10, Geant4 11.3.2
* 21 April 2025 --> Ubuntu Mate 24.04, Qt 6.9.0, CERN ROOT 6.32.10, Geant4 11.3.0
* 22 February 2025 --> Ubuntu Mate 24.04, Qt 6.8.2, CERN ROOT 6.32.10, Geant4 11.3.0

