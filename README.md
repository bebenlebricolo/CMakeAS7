![CMake Windows Nightly build](https://github.com/bebenlebricolo/CMake-AtmelStudio7-compatibility/workflows/CMake%20Windows%20Nightly%20build/badge.svg?branch=master)  ![CMake Windows Release build](https://github.com/bebenlebricolo/CMake-AtmelStudio7-compatibility/workflows/CMake%20Windows%20Release%20build/badge.svg)

# CMake AtmelStudio7 compatibility fork description

This fork of CMake aims to provide support for Atmel Studio 7 IDE (which is a rebranded Visual Studio 2017 version with a plugin sitting on top of it ).
It basically provides the ability to use a pre-existing Cmake build tree, directly using the raw avr toolchain for instance, and translate it into a valid Atmel Studio 7 project description.

[Here is a link to the original CMake README for common topics](README_CMAKE.md)

## How to use it
### Installing/Extracting Cmake AS7
First, download the [last stable release](https://github.com/bebenlebricolo/CMake-AtmelStudio7-compatibility/releases) from the release section of this repository.
Then, extract the package somewhere, and add the path to cmake.exe (found inside bin/ folder of the extracted release) into your User Path environment variable or System Path.
_Note: adding this build of cmake in any of your Path environment may mask the regular cmake build, **make sure they are not conflicting before updating your Path!**_

This build of cmake supports one more Generator, targeting Atmel Studio 7.0 IDE.
You can see a list of available Generators by writing the following command `cmake -G`.

_Note 2: my antivirus flags this cmake build when I try to use it for the first time. There is no virus packed inside but as this is the first time those builds are discovered by an antivirus, it has to be analysed first before letting you go with it. You can upload the release .zip file to [VirusTotal](https://www.virustotal.com/gui/) to perform a benchmark of antiviruses on the package, to ensure everything's clean about it!_

### Configure your AVR project
0. Navigate to your project directory and make sure a CMakeLists.txt is located in this folder.
1. Create a new directory - for instance `build` - and cd into it.
2. run the command : `cmake ../ -G "Atmel Studio 7.0"`. You can add your own definitions on top of it if you want so.
   _For instance, a more complete command could look like this :
   `cmake ../ -G "Atmel Studio 7.0" -DCMAKE_BUILD_TYPE=Release"`.
    If omitted, all configurations are parsed and they will all be used to generate AS7 project files._
3. Wait for it to finish testing your installation and generate files
  3.1. Sometimes, AS7 fails to start correctly (the IDE is launch in command line mode to build simple projects ; this is part of the tools validation process of CMake). This may cause the command line to hang.
  _Note : this is normal for the command line process to take a moment before responding ; CMake has to launch an external process, wait for its completion and return to normal mode, which could take a bit of time._
  **If such a case occurs, simply delete every generated files from the `build` directory, and start over.**
4. Generation is done, you can open the output .atsln file and start developping with Atmel Studio 7!

## Requirements
#### A working Atmel Studio 7 installation
To work properly, this fork needs the host computer to have a working installation of AtmelStudio7 IDE before attempting to generate project descriptions. This is part of the CMake's compiler and toolchain validation processes, in which CMake will try to probe build tools and compilers with simple test files and see if they are successfully built.

Furthermore, Atmel Studio 7 installation location is probed by CMake (both by Modules/**.cmake script files and CMake source code) and is used by the AtmelStudio7 Generator to retrieve important data for AtmelStudio7, for instance targeted device specification sheet.

#### A toolchain file!
Toolchain files are essential for CMake to understand we want to Cross Compile. Using toolchain files is mandatory for this fork to work as expected, otherwise CMake will try to compile for a desktop configuration and will inevitably fail!

Here is an example of toolchain file that I've customized for another project : [avr-gcc-toolchain.cmake](https://github.com/bebenlebricolo/LabBenchPowerSupply/blob/master/Firmware/Toolchain/avr-gcc-toolchain.cmake).
Then use it in your root CMakeLists.txt like so :
```CMake
cmake_minimum_required(VERSION 3.0)
# Use AVR GCC toolchain
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/Toolchain/avr-gcc-toolchain.cmake)
```

The aforementioned toolchain file declares very important variables such as :
```CMake
# see CMAKE_SYSTEM_NAME for cross compiling and Cmake system version
# Used by CMake to retrieve the right Modules from Cmake/Modules/Platform/${CMAKE_SYSTEM_NAME}
set( CMAKE_SYSTEM_NAME "BareMetal" )
set( CMAKE_SYSTEM_PROCESSOR AVR8 )
set( CMAKE_SYSTEM_VERSION "Generic" )
set( CMAKE_SYSTEM_VENDOR_NAME "Atmel" )
# Used by CMake to use the platform in order to test project's compilation when in the TryCompile() step.
set (CMAKE_GENERATOR_PLATFORM AVR8)
```
**So make sure to include the right toolchain file for your project !**

Also, you can check the [CMake-Atmel-toolchains](https://github.com/bebenlebricolo/CMake-Atmel-toolchains) project for updates and toolchains.

---

## Features
### Full compiler options and flags deduction (AVR GCC only at the moment)
* Compiler options and flags are parsed from `CMAKE_${LANG}_FLAGS` and `CMAKE_${LANG}_FLAGS_${CONFIG}`.
* Parsed options are then interpreted by a [compiler abstraction](Source/AtmelStudio7Generators/Compiler/cmAvrGccCompiler.h) (only avr gcc has the proper toolset to do so right now)
* Finally, parsed flags and options are stored in the right sections of AtmelStudio7 IDE properties pages ( [x] checkboxes for instance)
* Basic supported [options and flags kinds](Source/AtmelStudio7Generators/Compiler/Options) :
    * [Global Optimizations flags (-O0,....,-Ofast,etc)](Source/AtmelStudio7Generators/Compiler/Options/cmAvrGccOptimizationOption.h)
    * [Warning flags (-w, -Wall, -Wextra, -Werror)](Source/AtmelStudio7Generators/Compiler/Options/cmAvrGccWarningOption.h)
    * [Linker options and flags (-Wl,--relax,--gc-sections)](Source/AtmelStudio7Generators/Compiler/Options/cmAvrGccLinkerOption.h)
    * [Debugging flags (-g0, ..., -ggdb, -gdwarf, -g3, etc)](Source/AtmelStudio7Generators/Compiler/Options/cmAvrGccDebugOption.h)
    * [Symbol definitions (-D<symbol> and -D<symbol>=value) forms are supported](Source/AtmelStudio7Generators/Compiler/Options/cmAvrGccDefinitionOption.h)
    * [Language Standard flags (-std=c11, -std=c++17) ...](Source/AtmelStudio7Generators/Compiler/Options/cmAvrGccLanguageStandardOption.h)
    * [Machine options (specific to targeted core : -mmcu=<core>, -mrelax)](Source/AtmelStudio7Generators/Compiler/Options/cmAvrGccMachineOption.h)

### Targeted chip specification deduction:
* 8 bit AVR deduction (using the -mmcu compiler option)
* 32 bit AVR deduction (using the relevant -DXXX symbol passed to the toolchain)
* ARM32 SAM devices deduction (using the relevant -DXXX symbol passed to the toolchain)
Note : the chip specification deduction feature looks into the local AtmelStudio7 installation directory and windows register keys in order to retrieve specific data and configuration for each device. Devices are deduced using some naming conventions, the internal logic can be found within the [AS7DeviceResolver.cxx](Source/AtmelStudio7Generators/AS7Toolchains/AS7DeviceResolver.cxx).
Note 2 : device deduction is one of the first steps used by this project. Device deduction however does not mean that we can generate the full project description for every device that AtmelStudio7 supports at the moment, as it implies much more work in order to completely match the internal project file structure for every device.

### 8 bit AVR cores full options support: all specific compiler options are supported for AVR 8 devices
* ATmega series (e.g : ATmega328[P,PB], ATmega128)
* ATTiny series (e.g : ATtiny85, ATtiny10)
* ATautomotive series (e.g: ATautomotive)
* ATxmega series

### Full build tree support
#### Targets dependencies
Project's (aka CMake's "target" concept) dependencies are kept intact within AtmelStudio7 IDE, no extra manual referencing is required from the end user.
Every target is treated independently and its output file kind is preserved.
However, as AVR MCU's does not support dynamic libraries loading out of the box, only 2 kinds of products are available : executables and static libraries.

#### Build configurations
All build configurations described within the input CMake build tree are preserved and written into AtmelStudio7 project's descriptions.
Furthermore, as each build configuration serves a different purpose, their individual toolchain configuration is also kept intact and all options and flags are reported into the AtmelStudio7 project configuration (you would see it under Project>Properties>Toolchain)

#### Out of source build is supported
All files are linked in AtmelStudio7 as symlinks.
This allows to generate the AtmelStudio7 project within a build directory and to build your product within this build directory, to prevent polluting your source code with built files and byproducts.

1) cd to your project's folder
2) create a new build directory : `mkdir build`
3) enter the build directory and run the cmake command : `cmake ../ -G "Atmel Studio 7"`
4) if it went as expected, you'll end up with a `<myproject>.atsln` file somewhere in this folder
5) double click on this file to start AtmelStudio7.
6) you should be able to start building your project out of the box (otherwise, feel free to report it as a bug)

#### AtmelStudio7 auto-refresh when .atsln files or .cproj/.cppproj files are modified
Whenever you regenerate the whole project using this cmake fork, some files from AtmelStudio7 projects will be rewritten.
AtmelStudio7 IDE will detect this and refresh it's project descritption as a consequence. _This feature does not really come from this CMake fork but rather comes from Visual Studio behavior, but it's still a nice thing to know_

Note : _**changes made within AtmelStudio7 will be lost when the build tree is updated/regenerated, so make sure to report your changes inside your CMakeLists.txt files before regenerating your project!**_

---

# Compile cmake AS7 from sources
As this fork is only available for **Windows**, I encourage you to use [**Visual Studio 2019**](https://visualstudio.microsoft.com/fr/downloads/) to build it.
1. Get a version of [CMake](https://cmake.org/download/) (regular one)
2. Get a version of [Visual Studio 2019](https://visualstudio.microsoft.com/fr/downloads/)
3. Get a recent [git version](https://gitforwindows.org/)
4. Clone this repo somewhere `git clone https://github.com/bebenlebricolo/CMake-AtmelStudio7-compatibility.git"`
4.1. Init all submodules and update them : `git submodule init && git submodule update`
4. Cd into this repo and make a new `build` directory
5. Run the regular CMake tool : `cmake ../`. Normally, Cmake will be able to detect Visual Studio as the main build tool on your machine.
if not, try to force it like so instead: `cmake ../ -G "Visual Studio 16 2019"`.
_**Hint: use `cmake -G` without value to check the available generators**_
#### Visual Studio IDE build
6. Once generation is finished, open Visual Studio and load the CMake.sln file
7. Choose your build configuration, and build the whole solution (or only cmake project if you want so!)
8. To install cmake, select the INSTALL project in Visual Studio's Solution Explorer tab (See the _Note_ under)
#### Command line build tools with cmake
Alternatively, CMake can be built and installed using those 2 steps (once generation is done)
6. Run the following cmake build command from the `build` folder : `cmake --build . --config "Release"`
7. Run the following cmake install command if you want to install cmake right after building it:
`cmake --build . --target install --config "Release"`

##### Note about the installation directory
The install command will try to install cmake at `C:\Program Files(x86)\Cmake` by default, which may conflict with the regular cmake installation. To solve this issue, you can force cmake to install somewhere else, providing the cmake install directory at configuration time : `-DCMAKE_INSTALL_PREFIX=<yourpath>`.
[More information about CMAKE_INSTALL_PREFIX variable](https://cmake.org/cmake/help/latest/variable/CMAKE_INSTALL_PREFIX.html#variable:CMAKE_INSTALL_PREFIX)

For instance, configuration step could look like this instead :
`cmake ../ -DCMAKE_INSTALL_PREFIX=install -DCMAKE_BUILD_TYPE=Release`
This will set the right variable in Cmake cache to point the installation step to the build/install directory when the `install` target is built

# I want to understand the code for this fork (dev)
If, like me, you want to understand the code written specifically for this fork, here is a "snapshot" of the sections I've had to modify in order to add Atmel Studio 7 support to CMake.

* **Source/AtmelStudio7Generators** : Here lie the core components used by Cmake to generate a proper AtmelStudio7 compatible solution. Have a look to its [Readme.md](Source/AtmelStudio7Generators/Readme.md) for a more detailed overview.
* **Source/ExternalLibraries** : This folder is used to reference submodules as dependencies. It goes a bit out of the original CMake's philosophy (which is basically to reimplement everything). I though it was OK to take a bit of slack from the original philosophy and use external components that do the job just as well as a custom implementation. At the moment here are the project used :
  * [GoogleTest Unit testing framework](https://github.com/google/googletest). Used to implement and run unit tests easily, with a bit more support for conventional unit testing tools that what CTests actually provides.
  * [PugiXML](https://github.com/zeux/pugixml) : Used to generate XML files (AtmelStudio7 heavily relies on XML files to describe its projects [.cproj, .cppproj, .asmproj]).
* **Source/Utils** : This folder contains basic utils to perform very simple string manipulations. This arguably partially overlaps some custom tools already provided by CMake's project, but as CMake sometimes relies on dedicated cmString objects, I decided to make a STL compliant project instead. Another choice could have been to link with boost ...
* **Modules/Platform/BareMetal/Atmel** : Here are the modules used to "script" cmake's behavior while building the project's internal representation. Those modules are essentially used to determine the buid tools and compiler suites required in order select and validate the tools used to build a project.

## Testing strategy
