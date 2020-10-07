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
  static constexpr char* GeneratorName = "Atmel Studio 7.0";
  static constexpr char* SolutionFileExtension = ".atsln";
  static constexpr char* ProjectsFileExtension = ".cproj";
  static constexpr char* MinimumVisualStudioVersion = "10.0.40219.1";
  static constexpr char* VisualStudioLastVersion = "14.0.23107.0";

  /**
    @brief Lists available platforms supported by AtmelStudio
  */
  enum AvailablePlatforms
  {
    Unsupported, /**< Default value */
    AVR8,        /**< 8 bit AVR cores   */
    AVR32,       /**< 32 bit AVR cores */
    ARM          /**< ARM cores (for SAM devices) */
  };

  static std::map<AvailablePlatforms, std::string> PlatformMap;
  static std::string GetPlatform(AvailablePlatforms platform);
  static AvailablePlatforms GetPlatform(const std::string& name);

  AvailablePlatforms GetCurrentPlatform() const;

  static std::unique_ptr<cmGlobalGeneratorFactory> NewFactory();
  bool MatchesGeneratorName(const std::string& name) const;
  bool IsAtmelStudioInstalled() const;

  virtual ~cmGlobalAtmelStudio7Generator();

  void EnableLanguage(std::vector<std::string> const& languages, cmMakefile*,
                      bool optional) override;

  void Generate() override;

  // bool SetGeneratorPlatform(std::string const& p, cmMakefile* mf) override;

  /** Return true if the generated build tree may contain multiple builds.
      i.e. "Can I build Debug and Release in the same tree?" */
  bool IsMultiConfig() const override { return true; }

  /** Return true if building for Windows CE */
  virtual bool TargetsWindowsCE() const { return false; }

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

  // bool FindMakeProgram(cmMakefile*) override;

  // void ComputeTargetObjectDirectory(cmGeneratorTarget* gt) const override;

  bool IsVisualStudio() const override { return true; }

  class TargetSet : public std::set<cmGeneratorTarget const*>
  {
  };
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
  class Factory;
  friend class Factory;

  static constexpr AvailablePlatforms DefaultPlatform = AvailablePlatforms::AVR8;
  static constexpr char* ProjectConfigurationSectionName = "ProjectConfiguration";
  AvailablePlatforms CurrentPlatform = DefaultPlatform;

  // Set during OutputATSLNFile with the name of the current project.
  // There is one ATSLN file per project.
  std::string CurrentProject;
  std::string GetStartupProjectName(cmLocalGenerator const* root) const;

  cmGlobalAtmelStudio7Generator(cmake* cm,
                                const std::string& platformInGeneratorName);

  const char* GetIDEVersion() const { return "7.0"; }

  std::set<std::string> IsPartOfDefaultBuild(std::vector<std::string> const& configs,
                                             OrderedTargetDependSet const& projectTargets,
                                             cmGeneratorTarget const* target);

  bool IsDependedOn(OrderedTargetDependSet const& projectTargets,
                    cmGeneratorTarget const* gtIn);

  void WriteATSLNHeader(std::ostream& fout);

  std::string GetATSLNFile(cmLocalGenerator* const root) const;
  void OutputATSLNFile();
  void OutputATSLNFile(cmLocalGenerator* root,
                       std::vector<cmLocalGenerator*>& generators);

  void WriteATSLNFile(std::ostream& fout,
                      cmLocalGenerator* root,
                      std::vector<cmLocalGenerator*>& generators);

  void WriteProject(std::ostream& fout,
                    const std::string& name,
                    const std::string& path,
                    const cmGeneratorTarget* t);

  void WriteProjectDepends(std::ostream& fout, const std::string& name,
                           const std::string& path,
                           cmGeneratorTarget const* t);

  std::string ConvertToSolutionPath(const std::string& path);

  void WriteProjectConfigurations(std::ostream& fout,
                                  const std::string& name,
                                  cmGeneratorTarget const& target,
                                  std::vector<std::string> const& configs,
                                  const std::set<std::string>& configsPartOfDefaultBuild,
                                  const std::string& platformMapping = "");

  void WriteSolutionConfigurations(std::ostream& fout,
                                   std::vector<std::string> const& configs);

  void WriteATSLNGlobalSections(std::ostream& fout,
                                cmLocalGenerator* root);
  void WriteATSLNFooter(std::ostream& fout);

  void WriteTargetsToSolution(std::ostream& fout,
                              cmLocalGenerator* root,
                              OrderedTargetDependSet const& projectTargets);

  // void WriteTargetDepends(std::ostream& fout,
  //                        OrderedTargetDependSet const& projectTargets);

  void WriteTargetConfigurations(std::ostream& fout,
                                 std::vector<std::string> const& configs,
                                 OrderedTargetDependSet const& projectTargets);

  const char* ExternalProjectType(const std::string& location);

  void WriteExternalProject(std::ostream& fout,
                            const std::string& name, const std::string& path,
                            const char* typeGuid,
                            const std::set<BT<std::pair<std::string, bool>>>& dependencies);

  void WriteFolders(std::ostream& fout);
  void WriteFoldersContent(std::ostream& fout);
  std::map<std::string, std::set<std::string>> AtmelStudioFolders;

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
