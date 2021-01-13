![CMake Windows Nightly build](https://github.com/bebenlebricolo/CMake-AtmelStudio7-compatibility/workflows/CMake%20Windows%20Nightly%20build/badge.svg?branch=master)  ![CMake Windows Release build](https://github.com/bebenlebricolo/CMake-AtmelStudio7-compatibility/workflows/CMake%20Windows%20Release%20build/badge.svg)

# CMake AtmelStudio7 compatibility fork description

This fork of CMake aims to provide support for Atmel Studio 7 IDE (which is a rebranded Visual Studio 2017 version with a plugin sitting on top of it ).
It basically provides the ability to use a pre-existing Cmake build tree, directly using the raw avr toolchain for instance, and translate it into a valid Atmel Studio 7 project description.

[Here is a link to the original CMake README for common topics](README_CMAKE.md)

## Requirements
#### A working Atmel Studio 7 installation
To work properly, this fork needs the host computer to have a working installation of AtmelStudio7 IDE before attempting to generate project descriptions. This is part of the CMake's compiler and toolchain validation processes, in which CMake will try to probe build tools and compilers with simple test files and see if they are successfully built.

Furthermore, Atmel Studio 7 installation location is probed by CMake (both by Modules/**.cmake script files and CMake source code) and is used by the AtmelStudio7 Generator to retrieve important data for AtmelStudio7, for instance targeted device specification sheet.

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

#### AtmelStudio7 auto-refresh when .atsln files or .cproj/.cppproj files are modified
Whenever you regenerate the whole project using this cmake fork, some files from AtmelStudio7 projects will be rewritten.
AtmelStudio7 IDE will detect this and refresh it's project descritption as a consequence.
**However, changes made within AtmelStudio7 will be lost when the build tree is updated/regenerated, so make sure to report your changes inside your CMakeLists.txt files before regenerating your project!**

# Compile cmake AS7 from sources
As this fork is only available for windows, I encourage you to use Visual Studio 2019 to build it.
1) Get a version of CMake (regular one)
2) Get a version of Visual Studio 2019
3) clone this repo somewhere `git clone https://github.com/bebenlebricolo/CMake-AtmelStudio7-compatibility.git"`
3.1) init all submodules and update them : `git submodule init && git submodule update`
4) Cd into this repo and make a new `build` directory
5) Run the regular CMake tool : `cmake ../`. Normally, Cmake will be able to detect Visual Studio as the main build tool on your machine.
if not, try to force it like so instead: `cmake ../ -G "Visual Studio 16 2019`.
Use `cmake -G` without value to check the available generators
6) Once generation is finished, open Visual Studio and load the CMake.sln file
7) Choose your build configuration, and build the whole solution (or only cmake project if you want so!)
