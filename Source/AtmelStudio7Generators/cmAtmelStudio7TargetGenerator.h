/*
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3.0 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <array>
#include <iosfwd>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "cmGeneratorTarget.h"
#include "cmStringAlgorithms.h"
#include "cmGlobalVisualStudioGenerator.h"
#include "AS7ToolchainTranslator.h"


class cmComputeLinkInformation;
class cmCustomCommand;
class cmGeneratedFileStream;
class cmGlobalAtmelStudio7Generator;
class cmLocalAtmelStudio7Generator;
class cmMakefile;

/**
  * @brief forward declaring pugixml classes
  */
namespace pugi {
class xml_node;
class xml_doc;
}



// TODO : Move this function elsewhere : this method has little to do with TargetGenerator and is more related to XML manipulations than anything else.
/**
 * @brief Appends an inlined xml node to a parent node.
 * @param parent      :   xml parent node reference
 * @param node_name   :   node's name (<node_name>....</node_name>)
 * @param value       :   node's value
 */
static void AppendInlinedNodeChildPcData(pugi::xml_node& parent, const std::string& node_name, const std::string& value = "");

/**
 * @brief Describes an AtmelStudio7 Project (a.k.a CMake's Target)
*/
class AS7ProjectDescriptor
{
public:
  /**
    * @brief Describes the available project types supported in AtmelStudio7
  */
  enum class Type : uint8_t
  {
    cppproj = 0, /**< Target needs CPP support       */
    cproj,       /**< Target needs C support         */
    asmproj,     /**< Target needs Assembler support */
    maxval       /**< Used to determine the size of the embedded std::array*/
  } ProjectType;

  /**
   * @brief Used to pair a project file extension (e.g. cppproj) with its related language
  */
  struct Properties
  {
    Properties() = default;
    Properties(const std::string& _name,
               Type _type,
               const std::string& _language,
               const std::string& _compiler_vs_targets)
      : name(_name)
      , type(_type)
      , language(_language)
      , compiler_vs_targets(_compiler_vs_targets)
    {
    }
    std::string name; /**< Embeds its own name     */
    Type type;        /**< Embeds its project type */
    std::string language;
    std::string compiler_vs_targets;
  };

  /**
   * @brief Resolves a project type properties using its name as a key
   * @param[in] name  : name of the targeted project type
   * @return As7ProjectProperties structure or nullptr if it does not exist
  */
  static Properties* get_project_type_properties(const std::string& name);

  /**
   * @brief Resolves a project type properties using its enum version as a key
   * @param[in] name  : name of the targeted project type
   * @return As7ProjectProperties structure or nullptr if it does not exist
  */
  static Properties* get_project_type_properties(const Type type);

  /**
   * @brief Computes project file extension using its name.
   *
   * @param type    :  Project type using the enumerated value
   * @return computed extension string
   */
  static std::string get_extension(Type type);

private:
  static std::array<Properties, static_cast<uint8_t>(Type::maxval)> collection; /**< Stores an array of PropjectTypeProperties to link properties
                                                                                     alongside with the right projet type name                    */
};

/**
 * @brief Atmel Studio 7 Target Generator class used to parse C/C++ flags and generate AS7 project files according
 * to the Cmake project description.
*/
class cmAtmelStudio7TargetGenerator
{
public:
  cmAtmelStudio7TargetGenerator(cmGeneratorTarget* target,
                                cmGlobalAtmelStudio7Generator* gg);
  ~cmAtmelStudio7TargetGenerator();

  cmAtmelStudio7TargetGenerator(cmAtmelStudio7TargetGenerator const&) = delete;
  cmAtmelStudio7TargetGenerator& operator=(
    cmAtmelStudio7TargetGenerator const&) = delete;

  /**
   * @brief Entry point to generate the project file using the right formalism.
   * This method is called from the upper Local generator (cmLocalAtmelStudio7Generator) when generating
   * Build tree
  */
  void Generate();

private:

  /**
   * @brief Uses the toolchain translator to parse cmake flags from current solution.
   *
   * @param languages   :   list of enabled languages
   * @param upConfig    :   upper case build configuration
   * @return a list of flags
   */
  std::unordered_map<std::string, std::vector<std::string>> RetrieveCmakeFlags(const std::vector<std::string>& languages,
                                                                            const std::string& upConfig);
  /**
   * @brief Retrieves include directories for a given configuration, for a specific language.
   * @param config  :   build configuration used as a reference
   * @param lang    :   targeted language
   * @return a vector of includes, which can be empty in case of failure.
   */
  std::vector<std::string> GetIncludes(std::string const& config,
                                       std::string const& lang) const;
  /**
   * @brief Retrieves target defines for a given configuration, for a specific language.
   * @param config  :   build configuration used as a reference
   * @param lang    :   targeted language
   * @return a vector of defines, which can be empty in case of failure.
   */
  std::vector<std::string> GetDefines(std::string const& config,
                                       std::string const& lang) const;

private:
  bool InSourceBuild;
  std::vector<std::string> Configurations;  /**< Enabled build configurations collection                                                     */
  cmGeneratorTarget* const GeneratorTarget; /**< */
  cmMakefile* const Makefile;               /**< CMakeLists.txt target file which is used to describe this target                            */
  std::string const Platform;               /**< targeted Platform                                                                           */
  std::string const Name;                   /**< Target name                                                                                 */
  std::string const GUID;                   /**< This target's unique ID                                                                     */

  AvrToolchain::AS7ToolchainTranslator translator; /**< Used to parse compiler command line input and convert it to xml                      */


  // FIXME : This could be moved to the toolchain translator (AS7ToolchainTranslator) as
  // all information required to fill it up is available when using the toolchain translator !
  struct TargetedDevice_t
  {
    std::string name;                   /**< AtmelStudio compatible device name */
    std::string DFP_name;               /**< Device's targeted DFP name         */
    std::string version;                /**< Device's package version           */
    std::string mmcu_option;            /**< Device's mmcu option if it exists  */

    /**
     * @brief Constructs the DFP path relatively to Atmel Studio installation path.
     * @param   packs_path : packs folder path within Atmel Studio 7 installation folder (absolute path)
     * @return an absolute path to the device's DFP folder
     */
    std::string get_dfp_path(const std::string& packs_path) const;

    /**
     * @brief Retrieves the Device's DFP include directory using the packs path (absolute path).
     *
     * @param packs_path    :   absolute path of the packs folder inside AS7's installation folder
     * @return a relative path pointing to the include directory, using AS7 formalism.
     *      E.g : output = %24(PackRepoDir)\atmel\ATmega_DFP\1.2.209\include
     */
    std::string get_dfp_include_dir(const std::string& packs_path);

    /**
     * @brief Resolves DFP pack version using the absolute packs path as an input.
     * It lists all available package versions and chooses the highest one as the targeted version
     * @param packs_path    :   absolute path of the packs folder inside AS7's installation folder
     * @return the selected package version alone (directory name)
     *      E.g. : output is "1.2.36"
     */
    std::string resolve_version(const std::string& packs_path);
  } TargetedDevice;


  cmGlobalAtmelStudio7Generator* const GlobalGenerator; /**< Embeds a link to the GlobalGenerator (cmGlobalAtmelStudio7Generator)   */
  cmLocalAtmelStudio7Generator* const LocalGenerator;   /**< Embeds a link to the LocalGenerator (cmLocalAtmelStudio7Generator)     */

  // TODO : remove this method from here as it is not related to TargetGenerators. Put it in a util lib instead
  /**
   * @brief Converts CMake's StringRange type to a vector of std::strings.
   * @param     range : Cmake's StringRange object
   * @return a collection of strings
   */
  std::vector<std::string> ConvertBTStringRange(const cmBTStringRange& range) const;

  /**
   * @brief Builds a configuration group and attaches its xml representation to the parent node.
   *
   * This method retrieves compilation options given to the compiler using the configuration parameter.
   * It also looks for global options (CMAKE_CXX_FLAGS for instance) and appends the flags at the beginning of the
   * configuration-specific ones (e.g. CMAKE_CXX_FLAGS_RELEASE).
   * Then options are parsed by the compiler abstractions provided in cmAvrGccCompiler and the compiler options classes
   * and doubles are removed ; priorities are applied between specific options (e.g. -O0 takes over any other -Oxx option)
   *
   * @param parent      : parent xml node
   * @param build_type  : build configuration (e.g. Release, MinSizeRel, Debug, etc...)
   */
  void BuildConfigurationXmlGroup(pugi::xml_node& parent, const std::string& build_type);

  /**
   * @brief Builds the CompileItem group which is common to AtmelStudio and VisualStudio (tells to the IDE what resources to compile/include).
   * @param parent  :   parent xml node
   */
  void BuildCompileItemGroup(pugi::xml_node& parent);

  /**
   * @brief Builds the device property group which tells to Atmel Studio 7 which device is being targeted and
   * what toolchain needs to be used with it.
   *
   * Device is resolved using 2 inputs : the -mmcu option in case one is passed to the compiler or the appropriate
   * -D__MACRO__ define given to the compiler.
   * For both options, Device name is resolved using Atmel naming convention and supports
   * ATmega, AT90mega, ATtiny, ATxmega, ATsam (ARM32), AT32uc (AVR32) and ATautomotive chips
   *
   * @param parent      : parent xml node
   * @param target_name : project name (AS7), CMake terminology (Target)
   * @param lang        : language used to build this project (C, CXX, ASM)
   */
  void BuildDevicePropertyGroup(pugi::xml_node& parent, const std::string& target_name, const std::string& lang = "C");

  /**
   * @brief Builds the Atmel Studio simulator basic configuration.

   * * This is a mainly a default implementation for this simulator implementation as for now,
   * there is no means for the end user (i.e the one who writes the CMakeLists.txt) to give an explicit
   * configuration for the simulator
   * // TODO : investigate this, maybe we can provide macros and functions from a CMake module to enable Simulator configuration
   *
   * @param parent              : parent xml node
   * @param device_signature    : gives the device signature used with the simulator to atmega328p's signature
   * @param stimuli_filepath    : gives the path to the stimuli file, if it does exist
   */
  void BuildSimulatorConfiguration(pugi::xml_node& parent, const std::string& device_signature = "0x1E930B", const std::string& stimuli_filepath = "");

  /**
   * @brief Builds the project reference group for AS7 project file.
   *
   * This section is used by AS7 to link several projects together and to be able to retrieve
   * libraries for instance when necessary.
   * It is the way AS7 implements dependencies between projects
   *
   * @param parent  : parent xml node
   */
  void BuildProjectReferenceItemGroup(pugi::xml_node& parent);

  cmGlobalVisualStudioGenerator::OrderedTargetDependSet GetTargetDependencies() const;
};
