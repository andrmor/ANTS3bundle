# ANTS3bundle

* Particle simulation part is mostly ready
* Optical simulation part is in beta
* GUI is mostly ready
* Currently the effort is on refactoring the scripting methods and the help system

---

Latest tested configuration: 
* 18 June 2024 --> Ubuntu 22.04 Mate, Qt 6.7.1, Root 6.32.02 and Geant4 11.2.1

---

# Installation instructions for ants3 on Ubuntu 22.04 Mate

If you plan to set up a virtual machine, give it at least 50 Gb of disc space!

It is recommended to set up your keyboard according to your true location (e.g. Portuguese), but set your location as UK (e.g. London), otherwise you will have to configer decimal separator to "." yourself!

### General

sudo su      # and enter your password

apt -y update && apt -y upgrade

apt -y dist-upgrade

apt -y install git wget

apt -y install build-essential cmake

apt -y install libtbb-dev

apt -y install libeigen3-dev

apt -y install x11-apps

apt -y install libgl2ps-dev liblz4-dev

apt -y install python3-dev

apt -y install libxerces-c-dev freeglut3-dev libmotif-dev tk-dev libxpm-dev libxmu-dev libxi-dev

### CERN ROOT

* Check for the newest version numbers at https://root.cern/ and modify the line below

export VERSION=6.28.04

export ROOTTGZ=root_v$VERSION.Linux-ubuntu22-x86_64-gcc11.3.tar.gz

wget https://root.cern.ch/download/$ROOTTGZ

mkdir /opt/root$VERSION

tar -xzf $ROOTTGZ -C /opt/root$VERSION

rm $ROOTTGZ

### Geant4

* Check for the newest version numbers at https://geant4.web.cern.ch/ and modify the line below

export VERSION=11.1.1

wget https://github.com/Geant4/geant4/archive/v$VERSION.tar.gz

tar -xzf v$VERSION.tar.gz -C /opt

rm v$VERSION.tar.gz

mkdir /opt/geant4-$VERSION-build

cd /opt/geant4-$VERSION-build

#### Geant4 without Qt visualisation support (if you do not plan to run Geant4 standalone, see QtCreator section above)

cmake -DCMAKE_INSTALL_PREFIX=/opt/geant4-$VERSION-install -DGEANT4_USE_GDML=ON -DCMAKE_BUILD_TYPE=Release -DGEANT4_INSTALL_DATA=ON -DGEANT4_USE_OPENGL_X11=ON -DGEANT4_USE_XM=ON -DGEANT4_USE_QT=OFF -DGEANT4_BUILD_MULTITHREADED=OFF -DCMAKE_CXX_STANDARD=17 /opt/geant4-$VERSION

#### Geant4 with Qt visualisation support (in July 2023 still requres Qt5, see QtCreator section above)

* Modify the Qt directory in the following line!

cmake -DCMAKE_INSTALL_PREFIX=/opt/geant4-$VERSION-install -DGEANT4_USE_GDML=ON -DCMAKE_BUILD_TYPE=Release -DGEANT4_INSTALL_DATA=ON -DGEANT4_USE_OPENGL_X11=ON -DGEANT4_USE_XM=ON -DGEANT4_USE_QT=ON -DGEANT4_BUILD_MULTITHREADED=OFF -DCMAKE_PREFIX_PATH=/home/andr/Qt/5.15.2/gcc_64/lib/cmake -DCMAKE_CXX_STANDARD=17 /opt/geant4-$VERSION

#### The nest is the same for both options
* The number 8 in the next line is the number of cores used during compilation, modify for the adequte for your system

make -j8           

make install

rm -rf /opt/geant4-$VERSION-build

rm -rf /opt/geant4-$VERSION

* For some Geant4 version, there seems to be a bug which prevents Geant4 from finding its own datasets (there will be a compaint during execution about enviromental variables).
   * To fix this problem, open the file (modify the path according to the version) /opt/geant4-11.1.1-install/bin/geant4.sh and uncomment all the lines with the enviromental variables for the datasets, for example "export G4NEUTRONHPDATA=$GEANT4_DATA_DIR/G4NDL4.7".

---
### THE NEXT INSTRUCTIONS TO BE EXECUTED NOT WITH SUPERUSER!!!
---
Start a new terminal

### QtCreator

Download online Install qtcreator: see https://www.qt.io/download

* go to open source section, 
* then download "Qt online installer",
* allow execution for the downloaded file (checkbox on the file context menu using th right mouse click) and start it
* you will have to create an account to use the installer
* Go for "Custom installation"
    * QtCreator and the tools will be automatically selected, you only need to selected the Qt version
    * For example, in Qt 6.5.1, you have to select:
      * Desktop gcc 64-bit
      * Additional libraries / QWebSockets
      * Additional libraries / QWebEngine
    * Normally skip the following part, but if you also plan to use ants2 or Geant4's Qt-based GUI, install the following:
        * Qt 5.15.2
           * Desktop gcc 64-bit
           * Qt Script

### ANTS3 source code
* Create a directory for ants3 and open a terminal inside
* git clone https://github.com/andrmor/ANTS3bundle
   * for dev version, use: git clone --branch dev https://github.com/andrmor/ANTS3bundle
 
### Decimal separator

* Confirm that your system is using "." as the decimal separator. If not, use the following command and then restart Ubuntu:

sudo update-locale LC_NUMERIC="en_GB.UTF-8"

### Setting up the environment

* If still not done so, exit the sudo mode or start a new terminal
* Open .bashrc file in the home directory (it is invisible by default)
* Adjuct the paths according to the root and geant4 versions in the following lines!
   * export /opt/root63006/bin/thisroot.sh
   * export /opt/geant4-11.1.1-install/bin/geant4.sh
* Adjust the path for the alias to start the QtCreator
   * alias qt="/pathToQt/Qt/Tools/QtCreator/bin/qtcreator"
* Add the following two lines (seems to be optional now):
   * export LD_LIBRARY_PATH=\$LD_LIBRARY_PATH:/usr/local/lib
   * export PATH=\$PATH:/usr/include
* Only if you also installed Qt5 (not needed for Qt6!), adjust the path for the Qt directory and run:
   export LD_LIBRARY_PATH=/pathToQt/Qt/5.15.2/gcc_64/lib:\$LD_LIBRARY_PATH

### Next steps

* Start a new terminal and type qt to start QtCreator (only in this way the environmental variables will be configured properly!)
* Click "Open project..." and select meta.pro file in the ANTS3 source directory (ANTS3bundle/ants3bundle/meta.pro)
* Click "Configure project" button
* To begin the compilation (and start ANTS3 when it is done), click the green triangular button in the lower-left corner of the QtCeator window
