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
  static std::array<Properties,
                    static_cast<uint8_t>(Type::maxval)>
    collection; /**< Stores an array of PropjectTypeProperties to link properties
                            alongside with the right projet type name                    */
};

class cmAtmelStudio7TargetGenerator
{
public:
  cmAtmelStudio7TargetGenerator(cmGeneratorTarget* target,
                                cmGlobalAtmelStudio7Generator* gg);
  ~cmAtmelStudio7TargetGenerator();

  cmAtmelStudio7TargetGenerator(cmAtmelStudio7TargetGenerator const&) = delete;
  cmAtmelStudio7TargetGenerator& operator=(
    cmAtmelStudio7TargetGenerator const&) = delete;

  void Generate();

private:
  struct ToolSource
  {
    cmSourceFile const* SourceFile;
    bool RelativePath;
  };
  struct ToolSources : public std::vector<ToolSource>
  {
  };
  using ToolSourceMap = std::map<std::string, ToolSources>;
  ToolSourceMap Tools;

  std::string ConvertPath(std::string const& path, bool forceRelative);
  void WriteTargetGlobalProperties(pugi::xml_node& node);
  void WriteBuildConfigurations(pugi::xml_node& node);
  void WriteCompileGroup(pugi::xml_node& node);
  void WriteProjectReferenceGroup(pugi::xml_node& node);
  void WriteAS7CompilerTargetsProp(pugi::xml_node& node);

  std::vector<std::string> GetIncludes(std::string const& config,
                                       std::string const& lang) const;

private:
  AS7ProjectDescriptor projectDescriptor;
  bool InSourceBuild;
  std::vector<std::string> Configurations;  /**< Enabled build configurations collection                            */
  cmGeneratorTarget* const GeneratorTarget; /**< */
  cmMakefile* const Makefile;               /**< CMakeLists.txt target file which is used to describe this target   */
  std::string const Platform;               /**< targeted Platform                                                  */
  std::string const Name;                   /**< Target name                                                        */
  std::string const GUID;                   /**< This target's unique ID                                            */

  cmGlobalAtmelStudio7Generator* const GlobalGenerator;
  cmLocalAtmelStudio7Generator* const LocalGenerator;

  using UsingDirectories = std::set<std::string>;
  using UsingDirectoriesMap = std::map<std::string, UsingDirectories>;
  UsingDirectoriesMap AdditionalUsingDirectories;

  //void ClassifyAllConfigSources();
  //void ClassifyAllConfigSource(cmGeneratorTarget::AllConfigSource const& acs);

  using ConfigToSettings =
    std::unordered_map<std::string,
                       std::unordered_map<std::string, std::string>>;
  std::unordered_map<std::string, ConfigToSettings> ParsedToolTargetSettings;
  bool PropertyIsSameInAllConfigs(const ConfigToSettings& toolSettings,
                                  const std::string& propName);
  void ParseSettingsProperty(const std::string& settingsPropertyValue,
                             ConfigToSettings& toolSettings);
  std::string GetCMakeFilePath(const char* name) const;

  void AppendInlinedNodeChildPcData(pugi::xml_node& parent, const std::string& node_name, const std::string& value = "");
  void BuildConfigurationXmlGroup(pugi::xml_node& parent, const std::string& build_type);
  void BuildCompileItemGroup(pugi::xml_node& parent);
  void BuildDevicePropertyGroup(pugi::xml_node& parent, const std::string& target_name);
  void BuildSimulatorConfiguration(pugi::xml_node& parent, const std::string& device_signature = "0x1E930B", const std::string& stimuli_filepath = "");
};
