JamomaPureData
==============
[![Build Status](https://travis-ci.org/jamoma/JamomaPureData.svg?branch=master)](https://travis-ci.org/jamoma/JamomaPureData)
<a href="https://scan.coverity.com/projects/5514">
  <img alt="Coverity Scan Build Status"
       src="https://scan.coverity.com/projects/5514/badge.svg"/>
</a>

This is a port of [JamomaMax](https://github.com/jamoma/JamomaMax) library to [Puredata](http://puradata.info).
For reference, it's based on https://github.com/jamoma/JamomaMax/commit/957d817c508a69de06538cc9ac17a197279ff9fe.

Building
--------
You need CMake > 3.0 to build Jamoma and JamomaPureData.

There are several submodules that need to be updated before build.
Then the build process is straightforward :

    git clone https://github.com/jamoma/JamomaPureData.git --depth 1
    cd JamomaPureData
    git submodule update --init
    mkdir build 
    cd build 
    cmake ..
    make

Pd-vanilla is now a submodule of JamomaPuredata.
By default it will build against the last tested Pd version.

If you want to build against another version or if you are building for Windows platform and need a DLL to link to, then you can override the Pd source folder with `-DPD_MAIN_PATH=/path/to/pd/` option.

On Mac OS X, you will find the Pd source directory inside the application bundle, it should look like : `/Applications/Pd-0.46-6-64bit.app/Contents/Resources/`.

For more informations on CMake build system, please see the [Building with CMake](https://github.com/jamoma/Jamoma/wiki/Building-with-CMake) page on Jamoma wiki.

You may also be interessed in [How to debug JamomaPuredata on Mac OS X](https://github.com/jamoma/JamomaPureData/wiki/How-to-debug-JamomaPuredata-on-Mac-OS-X).

This have been tested on Ubuntu 14.04 64bit and Mac OS X 10.9 with Pd vanilla 0.46-6 64bit and Pd-extended 0.43-4.

Using
-----

To use this library with Pd, you have to tell Pd where to find it.
When you try to load an external object (an object which is not shipped with Pd), Pd search through its "search paths" to find a candidate.
By adding the Jamoma folder to the search path, Pd can find it.
To do so on Pd-extended go to Preferences and click `New` to add a new path to the list then select the Jamoma folder (in which you have jamoma.pd_darwin or jamoma.pd_linux or jamoma.dll).
And on Vanilla, you can do the same from Preferences->Path...

Then you first have to load the `[jamoma]` external itself which loads and initializes the Jamoma environment.
You have two options :
- put a `[jamoma]` object in your patch
or
- add `jamoma` to the library to load at startup.
To add a library at startup on Pd-extended, go to Preferences and add in the "Startup flag" (or something like that) field : `-lib jamoma`.
If you are using Pd Vanilla (and I encourage you to do so), go to Preferences->Startup, add a new and type `jamoma`.

If Jamoma loads it prints something like : 

~~~~
Jamoma for Pd version ..- - 88aea77 | jamoma.org
build on Apr 16 2015 at 22:06:34
~~~~

on the Pd's console.
If you don't see that, Jamoma is not loaded.
