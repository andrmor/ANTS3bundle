# ANTS3bundle

* GUI and scripting components are ready, collecting user suggestion for tweaking
* Particle simulation part is in beta
* Optical simulation part is in alpha

---

# Installation instructions for ants3 on Ubuntu 22.04 Mate

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
export VERSION=6.28.04     # check for the newest version at https://root.cern/

export ROOTTGZ=root_v$VERSION.Linux-ubuntu22-x86_64-gcc11.3.tar.gz

wget https://root.cern.ch/download/$ROOTTGZ

mkdir /opt/root$VERSION

tar -xzf $ROOTTGZ -C /opt/root$VERSION

rm $ROOTTGZ

### QtCreator
Install qtcreator: see https://www.qt.io/download
* go to open source section, 
* then download "Qt online installer",
* allow execution for the downloaded file and start it
* you will have to create an account to use the installer
* Go for "Custom installation"
    * QtCreator and the tools will be automatically selected, you only need to selected the Qt version
    * For example, in Qt 6.5.1, you have to select:
      * Desktop gcc 64-bit
      * Additional libraries / QWebSockets
      * Additional libraries / QWebEngine
    * If you also plan to use ants2 or Geant4's Qt-based GUI, install the following:
        * Qt 5.15.2
           * Desktop gcc 64-bit
           * Qt Script
   * Do NOT start QtCreator from the installation tool, only from the terminal!
   *   only in this way the environmental variables will be configured properly!

### Geant4
export VERSION=11.1.1         # see for version number https://geant4.web.cern.ch/

wget https://github.com/Geant4/geant4/archive/v$VERSION.tar.gz

tar -xzf v$VERSION.tar.gz -C /opt

rm v$VERSION.tar.gz

mkdir /opt/geant4-$VERSION-build

cd /opt/geant4-$VERSION-build

* Modify the Qt directory in the following line.
* If Geant4's GUI is not needed, set -DGEANT4_USE_QT=OFF and remove -DCMAKE_PREFIX_PATH=... compilation key from the line

cmake -DCMAKE_INSTALL_PREFIX=/opt/geant4-$VERSION-install -DGEANT4_USE_GDML=ON -DCMAKE_BUILD_TYPE=Release -DGEANT4_INSTALL_DATA=ON -DGEANT4_USE_OPENGL_X11=ON -DGEANT4_USE_XM=ON -DGEANT4_USE_QT=ON -DGEANT4_BUILD_MULTITHREADED=OFF -DCMAKE_PREFIX_PATH=/home/andr/Qt/5.15.2/gcc_64/lib/cmake -DCMAKE_CXX_STANDARD=17 /opt/geant4-$VERSION

make -j8

make install

rm -rf /opt/geant4-$VERSION-build

rm -rf /opt/geant4-$VERSION

### ants3 source code
* Create a directory for ants3 and open a terminal inside
* git clone https://github.com/andrmor/ANTS3bundle
   * for dev version, use: git clone --branch dev https://github.com/andrmor/ANTS3bundle

exit

### Setting the environment: THE NEXT IS NOT WITH SUPERUSER!!!

* Adjuct the paths according to the root and geant4 versions!

echo ". /opt/root6.28.04/root/bin/thisroot.sh" >> ~/.bashrc

echo ". /opt/geant4-11.1.1-install/bin/geant4.sh" >> ~/.bashrc
              #  
* Adjust the path for the QtCreator

echo "alias qt="/home/andr/Qt/Tools/QtCreator/bin/qtcreator"" >> ~/.bashrc
              #
* Adjust the path for the Qt directory in the third line

echo "export LD_LIBRARY_PATH=\$LD_LIBRARY_PATH:/usr/local/lib" >>  ~/.bashrc

echo "export PATH=\$PATH:/usr/include" >>  ~/.bashrc

echo "export LD_LIBRARY_PATH=/home/andr/Qt/5.15.2/gcc_64/lib:\$LD_LIBRARY_PATH" >>  ~/.bashrc
