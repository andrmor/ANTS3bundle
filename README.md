# ANTS3bundle

* GUI and scripting components are ready, collecting user suggestion for tweaking
* Particle simulation part is in beta
* Optical simulation part is in alpha

---

# Installation instructions for ants3 on Ubuntu 22.04 Mate

If you plan to set up a virtual machine, give it at least 50 Gb of disc space!

It is recommended to set up your location as UK (e.g. London), otherwise you will have to configer decimal separator to "." yourself!

### General

sudo su      # and enter your password

update-locale LANG=en_GB.UTF-8       # this is to ensure you use decimal separator "."

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

exit

---
### THE NEXT INSTRUCTIONS TO BE EXECUTED NOT WITH SUPERUSER!!!
---
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

## If you do NOT use a virtual machine:

* Adjuct the paths according to the root and geant4 versions!

echo ". /opt/root6.28.04/root/bin/thisroot.sh" >> ~/.bashrc

echo ". /opt/geant4-11.1.1-install/bin/geant4.sh" >> ~/.bashrc
                
* Adjust the path for the QtCreator

echo "alias qt="/home/andr/Qt/Tools/QtCreator/bin/qtcreator"" >> ~/.bashrc

echo "export LD_LIBRARY_PATH=\$LD_LIBRARY_PATH:/usr/local/lib" >>  ~/.bashrc

echo "export PATH=\$PATH:/usr/include" >>  ~/.bashrc

* Only if you also installed Qt5 (not needed for Qt6!), adjust the path for the Qt directory and run:

echo "export LD_LIBRARY_PATH=/home/andr/Qt/5.15.2/gcc_64/lib:\$LD_LIBRARY_PATH" >>  ~/.bashrc

## If you use a virtual machine:

* Adjuct the paths according to the root and geant4 versions, and the path for the QtCreator:

echo "alias qt=\\". /opt/root6.28.04/root/bin/thisroot.sh;. /opt/geant4-11.1.1-install/bin/geant4.sh;/home/andr/Qt/Tools/QtCreator/bin/qtcreator\\"" >> ~/.bashrc
              
echo "export LD_LIBRARY_PATH=\$LD_LIBRARY_PATH:/usr/local/lib" >>  ~/.bashrc

echo "export PATH=\$PATH:/usr/include" >>  ~/.bashrc

* Only if you also installed Qt5 (not needed for Qt6!), adjust the path for the Qt directory and run:

echo "export LD_LIBRARY_PATH=/home/andr/Qt/5.15.2/gcc_64/lib:\$LD_LIBRARY_PATH" >>  ~/.bashrc

### Next?

* Start a new terminal and type qt to start QtCreator IDE (only in this way the environmental variables will be configured properly!)
* Click "Open project..." and select meta.pro file in the ANTS3 source directory (ANTS3bundle/ants3bundle/meta.pro)
* Click "Configure project" button
* To start ANTS3 compilation click the green triangular button in the lower-left corner of the QtCeator window
