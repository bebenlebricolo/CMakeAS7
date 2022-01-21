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

#include <iosfwd>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "cmGlobalGenerator.h"
#include "cmGlobalGeneratorFactory.h"
#include "cmTargetDepend.h"

class cmGlobalGeneratorFactory;
class cmCustomCommand;
class cmGeneratorTarget;
class cmLocalGenerator;
class cmMakefile;
class cmake;

/** @class cmGlobalAtmelStudio7Generator
 *  @brief Base class for global Atmel Studio generators.
 *
 * cmGlobalAtmelStudio7Generator aims to generate Atmel Studio 7 IDE
 * Solution & Project files (build tree and files content is similar to those
 * of Visual Studio 10).
 *
 * @note A lot of code has been duplicated from VisualStudio generators,
 * however AtmelStudio's behavior is quite different to the classic
 * VisualStudio one. As a result, code was partially duplicated and brought
 * back from many derived classes of VisualStudioXX generator.
 */
class cmGlobalAtmelStudio7Generator : public cmGlobalGenerator
{
public:
  /**
   * @brief Collection of one-time definitions for AtmelStudio 7 IDE
   */
  static const char* GeneratorName;               /**< Generator's name is used to instantiate the generator using command line input (@see cmGlobalAtmelStudio7Generator::Factory)   */
  static const char* TruncatedGeneratorName;      /**< Same as generator's name, but truncated                                                                                        */
  static const char* SolutionFileExtension;       /**< Solution file extension for AtmelStudio7 IDE */
  static const char* MinimumVisualStudioVersion;  /**< Used to generate solution files and project files to indicate the minimum required for Visual Studio IDE)*/
  static const char* VisualStudioLastVersion;     /**< Gives the latest VisualStudio version supported by AtmelStudio7  */

  /**
    @brief Lists available platforms supported by AtmelStudio
  */
  enum class AvailablePlatforms
  {
    Unsupported, /**< Default value                 */
    AVR8,        /**< 8 bit AVR cores               */
    AVR32,       /**< 32 bit AVR cores              */
    ARM32        /**< ARM32 cores (for SAM devices) */
  };

  static std::map<AvailablePlatforms, std::string> PlatformMap; /**< Stores all available platforms alongside their name (much like a dictionary) */

  // NOTE : those converters could be implemented elsewhere, there is no need to keep it tied to this generator as they depict generic MCU cores
  /**
   * @brief Converts an AvailablePlatforms enum value into its std::string representation.
   * @param platform    : selected platform
   * @return a string evaluation of this platform enum
   */
  static std::string GetPlatform(AvailablePlatforms platform);

  /**
   * @brief Converts an string into its AvailablePlatforms enum representation.
   * @param platform    : selected platform
   * @return a string evaluation of this platform enum
   */
  static AvailablePlatforms GetPlatform(const std::string& name);

    /**
   * @brief Takes an "AvailablePlatform" and generates its compatible representation for ATSLN files
   * @param platform    : selected platform
   * @return a stripped version of platform's original string representation
   */
  static std::string StripPlatformForATSLN(const AvailablePlatforms& platform);

  /**
   * @brief Returns the current platform being used
   */
  AvailablePlatforms GetCurrentPlatform() const;

  /**
   * @brief Sets the platform for this generator.
   *
   * Note : if p is left empty (""), the default platform is selected, which is AVR8
   *
   * @param p   :   string representation of the selected platform
   * @param mf  :   input cmakefile
   * @return true (platform exists) or false (given platform name does not exist)
   */
  bool SetGeneratorPlatform(std::string const& p, cmMakefile* mf) override;

  /**
   * @brief Lists all supported languages for this generator.
   */
  enum class SupportedLanguages
  {
    Unsupported, /**< Default value, unsupported language*/
    C,           /**< C language */
    CPP,         /**< C++ language*/
    ASM          /**< Assembly language (specific to each architecture/processor)    */
  };

  /**
   * @brief Gives a more defined interface to use languages.
   * Stores information about supported file formats for C, C++ and Assembly languages
   */
  struct LangProp
  {
    LangProp()
      : lang(SupportedLanguages::Unsupported)
    {
    }
    LangProp(SupportedLanguages _lang, const std::vector<std::string>& _lang_src_str,
             const std::vector<std::string>& _lang_header_str)
      : lang(_lang)
      , lang_src_str(_lang_src_str)
      , lang_header_str(_lang_header_str)
    {
    }
    SupportedLanguages lang;                  /**< Enum value for the represented language */
    std::vector<std::string> lang_src_str;    /**< String representation of languages      */
    std::vector<std::string> lang_header_str; /**< String representation of languages      */
  };

  /**
   * @brief Checks if this Global generator supports the given language.
   *
   * @param lang : input language to be tested
   * @return true (language is supported) or false (language is not supported)
   */
  static bool SupportsLanguage(const SupportedLanguages lang);

  /**
   * @brief Checks if this Global generator supports the given language.
   *
   * @param lang : input language to be tested using its string representation
   * @return true (language is supported) or false (language is not supported)
   */
  static bool SupportsLanguage(const std::string& lang);

  /**
   * @brief Checks whether languages passed are supported or not.
   *
   * @param languages   :   a list of languages to be tested
   * @param mf          :   input cmMakefile (not used)
   * @return true (languages are supported), false (languages are not supported)
   */
  bool CheckLanguages(std::vector<std::string> const& languages, cmMakefile* mf) const override;

  /**
   * @brief Gives the global generator name.
   */
  std::string GetName() const override;

  /**
   * @brief Creates a Factory used to instantiate this GlobalGenerator.
   * GlobalGenerators are protected in a way that prevents usage of their constructor,
   * one has to use their factory instead (which performs several checks prior to instantiation) and
   * then the Global Generator is created on the heap.
   *
   * @return unique pointer to the new created factory
   */
  static std::unique_ptr<cmGlobalGeneratorFactory> NewFactory();


  /**
   * @brief Checks against OS'register keys to know whether AtmelStudio7 is installed on the system or not.
   * @return true (Atmel Studio 7 is installed) or false (not installed)
   */
  static bool IsAtmelStudioInstalled();

  /**
   * @brief Retrieves the AtmelStudio7 installation folder from OS's register keys.
   * @return the full path if any installation is found, empty otherwise
   */
  static std::string GetAtmelStudio7InstallationFolder();

  /**
   * @brief Creates a new local generator using the adequate CMakeLists.txt file representation matching the
   * targeted folder.
   * For instance, a local generator will be created for each new CMakeLists.txt file encountered in the build tree
   *
   * @param mf  : pointer to the adequate CMakeLists.txt representation class
   * @return a unique pointer to the new local generator (could be nullptr if something went wrong)
   */
  std::unique_ptr<cmLocalGenerator> CreateLocalGenerator(cmMakefile* mf) override;

  virtual ~cmGlobalAtmelStudio7Generator();

  /**
   * @brief Tries to determine system informations such as compiler identification, toolchain capabilities and features
   * @see cmGlobalGenerator::EnableLanguage for the heavylifting !
   *
   * This function is essentially used to determine if all tools required to interprete/build files for
   * a given language are present on the machine and if they can pass several tests to prove they can
   * perform the building task.
   * For instance, this method provides several ways to check compilers for any given language and checks if they can
   * compile simple snippets of code.
   * On specific systems, this method can also tests for the presence of very specific functionalities offered by the compiler
   * such as their support for pthread or other features.
   *
   * All the above work is done in the base class cmGlobalGenerator. This class only handles
   * Atmel Studio7 specific tasks
   *
   * Note : this method also uses Cmake's Modules files on a regular basis to extend its testing capabilities.
   * @see Cmake Modules/ folder of this repo
   *
   * @param languages   : list of languages used in this cmake project
   * @param mf          : CmakeLists.txt class representation (parsed by this class)
   * @param optional    : tells whether compiler and toolchain tests are optional or not
   */
  void EnableLanguage(std::vector<std::string> const& languages, cmMakefile* mf, bool optional) override;

  /**
   * @brief Starts the generation cascade all the way down to cmAtmelStudio7TargetGenerator which does
   * all the per-target work.
   *
   * Generates project files (a.k.a. Atmel Solution files) and target files (a.k.a. Atmel's project files)
   * Note : there is a noticeable terminology difference bewteen the CMake's world and the Atmel/Visual studio worlds
   */
  void Generate() override;


  // TODO : This implementation only relies on AtmelStudio7 but could be improved to really perform
  // tests on real toolchains directly instead of relying on a IDE to perform that kind of tasks
  /**
   * @brief Generates the build command to try out toolchain and determine compiler's sanity.
   *
   * This implementation bypasses Base class implementation and reroutes calls directly
   * to AtmelStudio7 instead of testing individual toolchains/compiler. AtmelStudio handles this
   * work for us instead.
   *
   * @param makeProgram : user preferered make program, defaults to msbuild
   * @param projectName : gives the project name used to test the toolchain
   * @param projectDir  : project directory, used to find the project on the system
   * @param targetNames : names of the targets referenced in the project description
   * @param config      : gives the build configuration
   * @param fast        : supposedly makes the build process faster (for instance make -jXX command ?)
   * @param jobs        : gives the number of allocated jobs for this task (see make -j<jobs number>)
   * @param verbose     : if this option is set to true, build tool will give more detailed output
   * @param makeOptions : other options passed to the build tool
   * @return    list of commands used to build each targets
   */
  std::vector<cmGlobalGenerator::GeneratedMakeCommand> GenerateBuildCommand(
    const std::string& makeProgram, const std::string& projectName,
    const std::string& projectDir, std::vector<std::string> const& targetNames,
    const std::string& config, bool fast, int jobs, bool verbose,
    std::vector<std::string> const& makeOptions) override;


  /**
    * @brief Return true if the generated build tree may contain multiple builds.
    *  i.e. "Can I build Debug and Release in the same tree?"
    */
  bool IsMultiConfig() const override { return true; }

  /** Disabled method */
  virtual bool TargetsWindowsCE() const { return false; }

  /** Disabled method */
  bool IsIncludeExternalMSProjectSupported() const override { return false; }

  /** Get encoding used by generator for generated source files
   */
  codecvt::Encoding GetMakefileEncoding() const override
  {
    return codecvt::ANSI;
  }

  codecvt::Encoding GetProjectFileEncoding() const
  {
    return codecvt::Encoding::UTF8;
  }

  /**
   * @brief Used to find the build tool used to make a AtmelStudio7 project.
   * In our case, the default build tool for this task is AtmelStudio7 itself
   * So as long as it is installed on the system, return true
   * @param mf  : (unused) CMakeLists.txt class representation
   * @return true (always)
   */
  bool FindMakeProgram(cmMakefile* mf) override;

  /**
   * @brief AtmelStudio7 is using VisualStudio under the hood.
   * so this method shall always return true
   * @return true (always)
   */
  bool IsVisualStudio() const override { return true; }

  /**
   * @brief defines a set of targets.
   */
  class TargetSet : public std::set<cmGeneratorTarget const*>
  {};

  class TargetCompare
  {
    std::string First;

  public:
    TargetCompare(std::string const& first)
      : First(first)
    {
    }
    bool operator()(cmGeneratorTarget const* l,
                    cmGeneratorTarget const* r) const;
  };
  class OrderedTargetDependSet;

  //! Lookup a stored GUID or compute one deterministically.
  std::string GetGUID(std::string const& name);

protected:
  class Factory;        /**< Factory used to instantiate the cmGlobalAtmelStudio7Generator          */
  friend class Factory; /**< Only this Factory can instantiate cmGlobalAtmelStudio7Generator class  */

  static std::vector<LangProp> SupportedLanguagesList;  /**< Lists all languages supported by AtmelStudio7 tool */
  static const AvailablePlatforms DefaultPlatform;      /**< Default platform will always be AVR8*/
  static const char* ProjectConfigurationSectionName;   /**< Used when writing Atmel Studio solution file */
  AvailablePlatforms CurrentPlatform = DefaultPlatform; /**< Gives the current platform for which the build system is being created */

  // Set during OutputATSLNFile with the name of the current project.
  // There is one ATSLN file per project.
  std::string CurrentProject; /**< Keeps track of this AtmelStudio solution (aka Cmake "project") name */

  /**
   * @brief Gives the name of the startup project (a.k.a Cmake "target").
   * Tries to retrieve the VS_STARTUP_PROJECT property for each makefiles
   * this is related to the set_target_properties(target1..target2 PROPERTIES VS_STARTUP_PROJECT)
   * Note : it appears that the "The startup-project is stored in a binary file, which is NOT generated by CMake"
   * -> https://stackoverflow.com/questions/7304625/how-do-i-change-the-startup-project-of-a-visual-studio-solution-via-cmake
   * @param root
   * @return the name of the startup project referenced in CMakeLists build tree
   */
  std::string GetStartupProjectName(cmLocalGenerator const* root) const;

  //TODO : investigate if we can resolve useful informations at the beginning when the Platform string is
  // given as a parameter to this constructor.
  /**
   * @brief Protected constructor of this class !
   * The only way to instantiate this class is by using the cmGlobalAtmelStudio7Generator::Factory
   * which has key friendship access to this class (this was done this way to provide advanced protection against
   * wild instantiation I guess, I had to follow its pattern anyway)
   *
   * @param cm                      : "Represents a cmake invocation[...]" with the whole cmake context
   * @param platformInGeneratorName : platform name (normally resolved later using compiler options)
   */
  cmGlobalAtmelStudio7Generator(cmake* cm, const std::string& platformInGeneratorName);

  /**
   * @brief Retrieves the IDE version.
   * @return IDE version
   */
  const char* GetIDEVersion() const { return "7.0"; }

  // TODO : documentation is really poor, would be nice if actually tells what's going on ...!
  /**
   * @brief Copied from a VisualStudioXX implementation (...) and adapted to AtmelStudio Generators needs.
   *
   * @param configs         : available build configurations
   * @param projectTargets  : set of project's targets
   * @param target          : currently evaluated target
   * @return
   */
  std::set<std::string> IsPartOfDefaultBuild(std::vector<std::string> const& configs,
                                             OrderedTargetDependSet const& projectTargets,
                                             cmGeneratorTarget const* target);

  /**
   * @brief Checks if another target depends on this one.
   *
   * @param projectTargets  : set of project's targets
   * @param gtIn            : currently evaluated target
   * @return  true (this target is a dependency of at least another one) false (no one depends on this target)
   */
  bool IsDependedOn(OrderedTargetDependSet const& projectTargets,
                    cmGeneratorTarget const* gtIn);

  /**
   * @brief Writes the appropriate header for the AtmelStudio 7 solution file.
   * The header essentially consists in a BOM and several requirement lines such
   * as the minimum VisualStudio required version
   * @param fout : output stream
   */
  void WriteATSLNHeader(std::ostream& fout);

  /**
   * @brief Retrieves the Atmel Studio 7 solution file path for a given local generator.
   *
   * @param root : points to the root localGenerator (local generator for the root CMakeLists.txt file)
   * @return atsln file path
   */
  std::string GetATSLNFile(cmLocalGenerator* const root) const;

  /**
   * @brief Writes out the solution file for this build tree.
   * Iterates over AS7 projects (a.k.a. Cmake's targets) and
   * add them up as reference in the output solution file.
   * It uses the very first localgenerator of its internal list to retrieve the root generator.
   *
   * Note : this implementation seems a bit cluncky when we read the doc, that's a pretty
   * strong hypothesis to consider the "root" generator
   */
  void OutputATSLNFile();

  /**
   * @brief Writes a solution file for the given collection of generators.
   * The first generator of the generators parameter is supposed to be the root one and is used
   * to generate the whole solution file
   *
   * @param root        : explicit root generator for this whole Cmake project
   * @param generators  : collection of generators referenced as "projects" in the final solution file
   */
  void OutputATSLNFile(cmLocalGenerator* root,
                       std::vector<cmLocalGenerator*>& generators);

  /**
   * @brief Writes solution file to an output stream.
   *
   * @param fout        : output stream
   * @param root        : root local generator
   * @param generators  : collection of all local generators for this project
   */
  void WriteATSLNFile(std::ostream& fout,
                      cmLocalGenerator* root,
                      std::vector<cmLocalGenerator*>& generators);

  /**
   * @brief Writes a single project reference to the Atmel Studio Solution file.
   *
   * @param fout          : output stream
   * @param display_name  : AS7 project's display name (as it will appear in the IDE)
   * @param relative_path : relative path from solution's root
   * @param target_gen    : target generator class for this AS7 project (aka Cmake target)
   */
  void WriteProject(std::ostream& fout,
                    const std::string& display_name,
                    const std::string& relative_path,
                    const cmGeneratorTarget* target_gen);

  /**
   * @brief Writes project (aka AS7 Solution) dependencies .
   *
   * @param fout        :   output stream
   * @param target_gen  :   solution's dependency target
   */
  void WriteProjectDepends(std::ostream& fout, cmGeneratorTarget const* target_gen);

  /**
   * @brief Converts the input path to "Windows" path using backslashes
   * @param path : input path
   * @return
   */
  std::string ConvertToSolutionPath(const std::string& path);

  // NOTE : selecting a platform to locally override a project platform may not be a good idea, we should remove this possibility
  /**
   * @brief Writes AS7 projects configurations for one given project.
   *
   * @param fout                        : output stream
   * @param name                        : project's name
   * @param target                      : target generator
   * @param configs                     : collection of build configurations (Release, Debug, etc.)
   * @param configsPartOfDefaultBuild   : list of default build configurations if any
   * @param platformMapping             : used to override platform configuration for this particular project
   */
  void WriteProjectConfigurations(std::ostream& fout,
                                  const std::string& name,
                                  cmGeneratorTarget const& target,
                                  std::vector<std::string> const& configs,
                                  const std::set<std::string>& configsPartOfDefaultBuild,
                                  const std::string& platformMapping = "");

  /**
   * @brief Writes available build configurations to this solution file.
   *
   * @param fout        : output stream
   * @param configs     : collection of build configurations
   */
  void WriteSolutionConfigurations(std::ostream& fout,
                                   std::vector<std::string> const& configs);

  /**
   * @brief Writes the global section for Atmel Studio7 solution file.
   *
   * @param fout    : output stream
   * @param root    : rool local generator
   */
  void WriteATSLNGlobalSections(std::ostream& fout,
                                cmLocalGenerator* root);

  /**
   * @brief Writes the footer of Atmel Studio 7 Solution file.
   *
   * @param fout    : output stream
   */
  void WriteATSLNFooter(std::ostream& fout);

  /**
   * @brief Writes all targets (aka AS7 projects) to solution as references.
   *
   * @param fout            : output stream
   * @param root            : root local generator
   * @param projectTargets  : set of targets
   */
  void WriteTargetsToSolution(std::ostream& fout,
                              cmLocalGenerator* root,
                              OrderedTargetDependSet const& projectTargets);

  /**
   * @brief Writes build configuration for all targets.
   *
   * @param fout            : output stream
   * @param configs         : list of build configurations
   * @param projectTargets  : list of targets for this Cmake project (AS7 solution)
   */
  void WriteTargetConfigurations(std::ostream& fout,
                                 std::vector<std::string> const& configs,
                                 OrderedTargetDependSet const& projectTargets);

  // TODO : Enable folder organisation if possible (like VS "filters")
  //void WriteFolders(std::ostream& fout);
  //void WriteFoldersContent(std::ostream& fout);
  // std::map<std::string, std::set<std::string>> AtmelStudioFolders;

  using UtilityDependsMap = std::map<cmGeneratorTarget const*, std::string>;
  UtilityDependsMap UtilityDepends;
};

class cmGlobalAtmelStudio7Generator::OrderedTargetDependSet
  : public std::multiset<cmTargetDepend, cmGlobalAtmelStudio7Generator::TargetCompare>
{
  using derived = std::multiset<cmTargetDepend, cmGlobalAtmelStudio7Generator::TargetCompare>;

public:
  using TargetDependSet = cmGlobalGenerator::TargetDependSet;
  using TargetSet = cmGlobalAtmelStudio7Generator::TargetSet;
  OrderedTargetDependSet(TargetDependSet const&, std::string const& first);
  OrderedTargetDependSet(TargetSet const&, std::string const& first);
};
