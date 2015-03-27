JamomaPureData
==============
This is a port of [JamomaMax](https://github.com/jamoma/JamomaMax) library to [Puredata](http://puradata.info).

Building
--------
You need CMake > 3.0 to build Jamoma and JamomaPureData.

Concerning the build, the best is to start with [Jamoma](https://github.com/jamoma/Jamoma) umbrella repository in the `dev` branch.
Then initialize submodules Core and Implementations/PureData (and others if you want).
Then in the `Implementation/Puredata` repository, initialize and update submodule to grab the CicmWrapper.

This could be sumup like this :

~~~~
git clone -b dev https://github.com/jamoma/Jamoma
cd Jamoma
git submodule init
git submodule update Core
git submodule update Implementations/PureData
cd  Implementations/PureData
git submodule init
git submodule update
~~~~

Then you are ready to build JamomaCore and JamomaPuredata. From the root of the Jamoma umbrella repo : 

~~~~
mkdir build
cd build 
cmake ..
make
~~~~

This have been tested on Ubuntu 14.04 64bit.


