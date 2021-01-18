# AtmelStudio7Generators library
This library packs all the required components for this fork to be able to generate valid project descriptions for Atmel Studio 7 IDE.

## Components listing

#### Cmake generators suite
Those generators are used by CMake in order to correctly interprete CMakeLists.txt files which describe a complete build tree.
The following generators for AtmelStudio7 heavily relies on the VisualStudio[7,71,8,10] implementations ; a lot of code was duplicated in order to ~~"get inspiration"~~ speed up development times and as **AtmelStudio7** relies on **Visual Studio 2015**, it seemed to be a "smart" move to borrow some code from the other **Visual Studio XX** generators.
* **cmGlobalAtmelStudio7Generator** : single Global generator instanciated by CMake when parsing the root CMakeLists.txt.
* **cmLocalAtmelStudio7Generator** : single Local generator instanciated by CMake each time a "subdirectory" CMakeLists.txt is found.
* **cmAtmelStudio7TargetGenerator** : unique representation of a CMake "target", aka AtmelStudio7 "Project" concept.

#### Atmel Studio 7 toolchain abstractions
Those files are meant to provide a representation of how AtmelStudio7 builds its support for the various toolchains available. The following files then provide a layer of abstraction, trying to match as closely as possible the tabs found under a `"Project>Properties>Toolchain"` tabs of AtmelStudio7.

**AS7Toolchains/**
* **AS7DeviceResolver** : A series of tools used to guess a device name using the naming convention of AtmelStudio7. For instance, if the `atmega328p` is given as an input to the AS7DeviceResolver, we might be able to determine it's core (avr8), its DFP (ATmega_DFP), and various other properties such as its flash size and other parameters.
* **AS7ToolchainTranslator** : A class whose role is to translate a compiler representation (see [Compiler/cmAvrGccCompiler](Compiler/cmAvrGccCompiler)) into a valid [AvrGCC8Toolchain](AS7Toolchains/AvrGCC8Toolchain).
* **AvrGCC8Toolchain** : a collection of structures and classes used to depict the built-in support of Compiler's parameters and flags from within AtmelStudio7IDE. These class also provide some functionalities to generate XML descriptions used to build the final AS7 project description such as a `.cproj` for instance.

#### AvrGcc compiler abstractions
Those classes provide a very light front end abstraction to **AvrGcc** compiler at the moment and is able to parse options and flags ~~almost~~ like the real one will do. It is independant from any AtmelStudio7 representation and solely aims to parse real compiler options and categorize them correctly.
Another cool feature is that this compiler abstraction is able to apply priorities over some flags and optimizations, in order to remove doubles or filter out conflicting options (for instance `-O0` and `-O3` are conflicting options. In that special case, the `-O0` will prevail).

**Compiler/**
* **cmAvrGccCompiler** : the AvrGcc compiler abstraction used to parse incoming flags and options written into the original CMakeLists.txt build tree.
* **Options/**
  * **cmAvrGccCompilerOption** : A generic compiler option with no special features nor support
  * **cmAvrGccDebugOption** : Compiler option dedicated to parsing `-g..` flags
  * **cmAvrGccDefinitionOption** : Compiler option dedicated to parsing `-D..=..` flags. Both definitions **with** and **without** values are supported
  * **cmAvrGccLanguageStandardOption** : Compiler option dedicated to parsing `-std=...` flags. Use pattern matchin to identify language and standard kind (iso, c/c++)
  * **cmAvrGccMachineOption** : Compiler option dedicated to parsing `-m..` flags with support for options **with** and **without** values e.g. : `-mmcu=atmega328p`
  * **cmAvrGccLinkerOption** : Compiler option dedicated to parsing `-Wl,..` flags. Compacted options are also supported. e.g. : `-Wl,--gc-sections,--relax`
  * **cmAvrGccWarningOption** : Compiler option dedicated to parsing `-W..` flags. E.g : `-Wall -Wextra -Werror -w` are supported
  * **cmAvrGccOptimizationOption**: Compiler option dedicated to parsing `-O..` flags.
