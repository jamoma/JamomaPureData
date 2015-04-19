JamomaPureData
==============
This is a port of [JamomaMax](https://github.com/jamoma/JamomaMax) library to [Puredata](http://puradata.info).
For reference, it's based on https://github.com/jamoma/JamomaMax/commit/957d817c508a69de06538cc9ac17a197279ff9fe.

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
git submodule update --init
git submodule foreach git checkout dev
git submodule foreach git pull
cd  Implementations/PureData
git submodule update --init
~~~~

Then you are ready to build JamomaCore and JamomaPuredata. From the root of the Jamoma umbrella repo : 

~~~~
mkdir build
cd build 
cmake -DBUILD_JAMOMAPD:STRING=True ..
make
~~~~

This have been tested on Ubuntu 14.04 64bit.


