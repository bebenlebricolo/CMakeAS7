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

#include "cmAtmelStudio7TargetGenerator.h"

#include <algorithm>
#include <iterator>
#include <set>

#include <cm/memory>
#include <cm/string_view>
#include <cm/vector>
#include <cmext/algorithm>

#include "cmAvrGccMachineOption.h"
#include "cmGeneratedFileStream.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalAtmelStudio7Generator.h"
#include "cmLinkLineDeviceComputer.h"
#include "cmLocalAtmelStudio7Generator.h"
#include "cmMakefile.h"
#include "cmSourceFile.h"
#include "cmStringAlgorithms.h"
#include "cmStringUtils.h"
#include "cmSystemTools.h"

#include "AS7DeviceResolver.h"
#include "AS7ToolchainTranslator.h"
#include "pugixml.hpp"

#define CMAKE_CHECK_BUILD_SYSTEM_TARGET "ZERO_CHECK"

cmAtmelStudio7TargetGenerator::cmAtmelStudio7TargetGenerator(
  cmGeneratorTarget* target, cmGlobalAtmelStudio7Generator* gg)
  : GeneratorTarget(target)
  , Makefile(target->Target->GetMakefile())
  , Platform(gg->GetPlatform(gg->GetCurrentPlatform()))
  , Name(target->GetName())
  , GUID(gg->GetGUID(target->GetName()))
  , GlobalGenerator(gg)
  , LocalGenerator((cmLocalAtmelStudio7Generator*)target->GetLocalGenerator())
{
  this->Configurations = this->Makefile->GetGeneratorConfigs(cmMakefile::ExcludeEmptyConfig);
  this->InSourceBuild = (this->Makefile->GetCurrentSourceDirectory() == this->Makefile->GetCurrentBinaryDirectory());
}

cmAtmelStudio7TargetGenerator::~cmAtmelStudio7TargetGenerator()
{
}

static AS7ProjectDescriptor::Type computeProjectFileExtension(cmGeneratorTarget const* t)
{
  std::set<std::string> languages;
  std::string config;
  t->GetLanguages(languages, config);

  // C++ gets the priority
  if (languages.find("CXX") != languages.end()) {
    return AS7ProjectDescriptor::Type::cppproj;
  }

  // Then comes the C language
  if (languages.find("C") != languages.end()) {
    return AS7ProjectDescriptor::Type::cproj;
  }

  // Finally, if the only last language is assembly
  if (languages.find("ASM") != languages.end()) {
    return AS7ProjectDescriptor::Type::asmproj;
  }

  // Defaults to C projects
  return AS7ProjectDescriptor::Type::cproj;
}

void cmAtmelStudio7TargetGenerator::Generate()
{
  // Retrieve project file extension
  const AS7ProjectDescriptor::Type ProjectFileType = computeProjectFileExtension(this->GeneratorTarget);
  const std::string ProjectFileExtension = AS7ProjectDescriptor::get_extension(ProjectFileType);

  // Tell the global generator the name of the project file
  this->GeneratorTarget->Target->SetProperty("GENERATOR_FILE_NAME", this->Name);
  this->GeneratorTarget->Target->SetProperty("GENERATOR_FILE_NAME_EXT", ProjectFileExtension);

  std::string path = cmStrCat(this->LocalGenerator->GetCurrentBinaryDirectory(), '/', this->Name, ProjectFileExtension);
  cmGeneratedFileStream BuildFileStream(path);
  const std::string PathToProjectFile = path;
  BuildFileStream.SetCopyIfDifferent(true);

  // Output project file is an XML file using default UTF-8 encoding
  pugi::xml_document doc;
  // Prepend delcaration node will look like this : <?xml version="1.0" encoding="utf-8"?>
  // This is the implementation of XML Byte Order Mask (BOM) for UTF-8 encoding : https://en.wikipedia.org/wiki/Byte_order_mark
  pugi::xml_node decl = doc.prepend_child(pugi::node_declaration);
  decl.append_attribute("version") = "1.0";
  decl.append_attribute("encoding") = "utf-8";

  pugi::xml_node project_node = doc.append_child(pugi::node_element);
  project_node.set_name("Project");
  project_node.append_attribute("DefaultTargets") = "Build";
  project_node.append_attribute("xmlns") = "http://schemas.microsoft.com/developer/msbuild/2003";
  project_node.append_attribute("ToolsVersion") = "14.0";

  // Device name is resolved from compiler options
  BuildDevicePropertyGroup(project_node, this->Name);

  // Iterate over build configurations such as Release, Debug, etc. and write their dedicated descriptions
  // Based on compiler options
  std::vector<cmGeneratorTarget::AllConfigSource> const& sources = this->GeneratorTarget->GetAllConfigSources();
  for (std::string const& config : this->Configurations) {
    std::string configUpper = cmSystemTools::UpperCase(config);
    BuildConfigurationXmlGroup(project_node, config);
  }

  // Compile item group which lists sources to be built as part of this target
  BuildCompileItemGroup(project_node);

  // Add projects references
  BuildProjectReferenceItemGroup(project_node);

  // Last node is dedicated to atmelstudio specific targets
  pugi::xml_node import_projects_node = project_node.append_child("Import");
  import_projects_node.append_attribute("Project") = R"($(AVRSTUDIO_EXE_PATH)\\Vs\\Compiler.targets)";

  doc.save(BuildFileStream, "  ", (pugi::format_indent | pugi::format_write_bom), pugi::xml_encoding::encoding_utf8);
  BuildFileStream.Close();
}

void AppendInlinedNodeChildPcData(pugi::xml_node& parent, const std::string& node_name, const std::string& value)
{
  pugi::xml_node node = parent.append_child(node_name.c_str());
  if (0 != value.size()) {
    pugi::xml_node child = node.append_child(pugi::node_pcdata);
    child.set_value(value.c_str());
  }
}

std::vector<std::string> cmAtmelStudio7TargetGenerator::GetIncludes(const std::string& config, const std::string& lang) const
{
  std::vector<std::string> includes;
  this->LocalGenerator->GetIncludeDirectories(includes, this->GeneratorTarget,
                                              lang, config);
  for (std::string& i : includes) {
    i = cmutils::strings::replace(i, '/', '\\');
  }
  return includes;
}

std::vector<std::string> cmAtmelStudio7TargetGenerator::GetDefines(const std::string& config, const std::string& lang) const
{
  std::vector<std::string> out;
  std::set<std::string> probed_defines;
  this->LocalGenerator->GetTargetDefines(this->GeneratorTarget, config, lang, probed_defines);

  for (const std::string& define : probed_defines) {
    std::string def = cmutils::strings::replace(define, '/', '\\');
    out.push_back(def);
  }
  return out;
}

std::vector<std::string> cmAtmelStudio7TargetGenerator::ConvertStringRange(const cmStringRange& range) const
{
  std::vector<std::string> out;
  for (auto& val : range) {
    out.push_back(val);
  }
  return out;
}

// TODO : this util function could be implemented elsewhere, for instance in the cmAvrGccLanguageStandardOption or in a more generic place
static std::string encode_lang_std(const std::string& std_version, const std::string& lang, bool use_gnu_standard = false)
{
  std::string out = "-std=";

  if (use_gnu_standard) {
    out += "gnu";
  } else {
    out += "c";
  }

  if (lang == "CXX") {
    out += "++";
  }

  out += std_version;
  return out;
}

std::unordered_map<std::string, std::vector<std::string>> cmAtmelStudio7TargetGenerator::RetrieveCmakeFlags(const std::vector<std::string>& languages,
                                                                                                            const std::string& upConfig)
{
  std::unordered_map<std::string, std::vector<std::string>> out;
  std::string linker_variable_name = "CMAKE_EXE_LINKER_FLAGS";

  // Extract all compiler options, parse them and use them in the final XML file
  for (auto& lang : languages) {
    std::vector<std::string> all_flags;
    std::string flag_variable_name = "CMAKE_" + lang + "_FLAGS";
    std::string flag_variable_config_name = flag_variable_name + "_" + upConfig;
    std::string lang_standard_varname = "CMAKE_" + lang + "_STANDARD";

    cmProp lang_flags_base = this->Makefile->GetDefinition(flag_variable_name);
    cmProp lang_flags_config = this->Makefile->GetDefinition(flag_variable_config_name);
    cmProp lang_standard_def = this->Makefile->GetDefinition(lang_standard_varname);

    auto compile_definitions = ConvertStringRange(this->Makefile->GetCompileDefinitionsEntries());
    auto compile_options = ConvertStringRange(this->Makefile->GetCompileOptionsEntries());

    std::vector<std::string> base_flags;
    std::vector<std::string> config_flags;

    if (lang_flags_base != nullptr) {
      base_flags = cmutils::strings::split(*lang_flags_base);
    }

    if (lang_flags_config != nullptr) {
      config_flags = cmutils::strings::split(*lang_flags_config);
    }

    if (lang_standard_def != nullptr) {
      all_flags.push_back(encode_lang_std(*lang_standard_def, lang, false));
    }

    // Merge all flags within one big vector
    all_flags.insert(all_flags.end(), base_flags.begin(), base_flags.end());
    all_flags.insert(all_flags.end(), config_flags.begin(), config_flags.end());
    all_flags.insert(all_flags.end(), compile_definitions.begin(), compile_definitions.end());
    all_flags.insert(all_flags.end(), compile_options.begin(), compile_options.end());

    auto last_elem = std::unique(all_flags.begin(), all_flags.end());
    all_flags.resize(std::distance(all_flags.begin(), last_elem));

    out[lang] = all_flags;
  }
  return out;
}

void cmAtmelStudio7TargetGenerator::BuildConfigurationXmlGroup(pugi::xml_node& parent, const std::string& build_type)
{
  // Clears translator before adding data into it
  translator.toolchain.avrgcc.clear();
  translator.toolchain.avrgcccpp.clear();
  translator.toolchain.assembler.clear();
  translator.toolchain.linker.clear();
  // Don't clear the archiver member of translator.toolchain because it is actually needed by Atmel Studio to be able to
  // Pack built libraries into their proper files.
  // If -r flag is not present in archiver_flags, Atmel Studio fails with the "make : no rules to make <target> needed by all:"

  pugi::xml_node property_group_node = parent.append_child("PropertyGroup");
  std::string conditionnal_str = " '$(Configuration)' == '" + build_type + "' ";
  property_group_node.append_attribute("Condition").set_value(conditionnal_str.c_str());

  // Get languages
  std::vector<std::string> enabledLanguages;
  this->GlobalGenerator->GetEnabledLanguages(enabledLanguages);

  // Parse flags
  const std::string upConfig = cmutils::strings::to_uppercase(build_type);
  //auto CMAKE_CXX_FLAGS = this->Makefile->GetDefinition("CMAKE_CXX_FLAGS");

  // Extract all flags for this config : CMAKE_${LANG}_FLAGS  + CMAKE_${LANG}_FLAGS_${CONFIG}
  // Works for multiple languages {C,CXX}
  std::unordered_map<std::string, std::vector<std::string>> all_flags = RetrieveCmakeFlags(enabledLanguages, upConfig);

  // Parse flags for all languages
  for (auto& flags : all_flags) {
    // Parse incoming flags
    translator.parse(flags.second, flags.first);
  }

  // Open the Toolchain Settings node
  pugi::xml_node toolchain_settings_node = property_group_node.append_child("ToolchainSettings");
  std::string avr_gcc_node_name = "AvrGcc";
  if (std::find(enabledLanguages.begin(), enabledLanguages.end(), "CXX") != enabledLanguages.end()) {
    avr_gcc_node_name = "AvrGccCpp";
  }
  pugi::xml_node avr_gcc_node = toolchain_settings_node.append_child(avr_gcc_node_name.c_str());

  // NOTE : this is a "default" setting for Release config which might be annoying, consider removing this if need be
  // Classic default symbols for Release configuration
  if (build_type != "Release") {
    translator.toolchain.avrgcc.symbols.def_symbols.push_back("NDEBUG");
    translator.toolchain.avrgcccpp.symbols.def_symbols.push_back("NDEBUG");
  }

  // Handle include paths
  std::string as7_installation_folder = this->GlobalGenerator->GetAtmelStudio7InstallationFolder();
  std::string dfp_dir = TargetedDevice.get_dfp_path(as7_installation_folder + "\\packs\\");
  TargetedDevice.version = AS7DeviceResolver::get_max_packs_version(dfp_dir);

  std::string dfp_include_dir = TargetedDevice.get_dfp_include_dir(as7_installation_folder + "\\packs\\");

  translator.toolchain.avrgcc.directories.include_paths.push_back(dfp_include_dir);
  translator.toolchain.avrgcccpp.directories.include_paths.push_back(dfp_include_dir);

  // This is the only way the toolchain translator could know about those includes as
  // they are not parsed from compiler options alone.
  for (const auto& lang : enabledLanguages) {
    auto includesList = this->GetIncludes(build_type, lang);
    auto definesList = this->GetDefines(build_type, lang);

    for (const auto& singleInclude : includesList) {
      if (lang == "CXX") {
        translator.toolchain.avrgcccpp.directories.include_paths.push_back(singleInclude);
      } else if (lang == "C") {
        translator.toolchain.avrgcc.directories.include_paths.push_back(singleInclude);
      }
    }

    for (const auto& define : definesList) {
      if (lang == "CXX") {
        translator.toolchain.avrgcccpp.symbols.def_symbols.push_back(define);
      } else if (lang == "C") {
        translator.toolchain.avrgcc.symbols.def_symbols.push_back(define);
      }
    }
  }

  // Configure linker
  // libm is always present in projects, so add it by default.
  // NOTE : this default behavior could be annoying as well, provide a way to NOT include libm everytime
  translator.toolchain.linker.libraries.libraries.push_back("libm");
  cmGlobalVisualStudioGenerator::OrderedTargetDependSet target_dependencies = GetTargetDependencies();

  // Resolve project's dependencies
  for (cmGeneratorTarget const* dependent_target : target_dependencies) {
    if (!dependent_target->IsInBuildSystem()) {
      continue;
    }
    cmLocalGenerator* lg = dependent_target->GetLocalGenerator();
    translator.toolchain.linker.libraries.libraries.push_back(dependent_target->GetName());

    // Including built libraries with the right configuration for automatic linking
    // features
    if (lg != nullptr) {
      std::string build_directory = lg->GetCurrentBinaryDirectory() + "/" + build_type;
      translator.toolchain.linker.libraries.search_path.push_back(cmutils::strings::replace(build_directory, "/", "\\"));
    }
  }
  translator.toolchain.assembler.general.include_path.push_back(dfp_include_dir);

  // Finally generate the xml project file
  translator.generate_xml(avr_gcc_node);
}


static void add_source_compile_node(pugi::xml_node& item_group_node, const cmGeneratorTarget::AllConfigSource& s)
{
  const char* toolname = "Compile";
  pugi::xml_node compile_node = item_group_node.append_child(toolname);
  std::string path = s.Source->GetFullPath().c_str();

  // Convert regular slashes to Windows backslashes
  std::replace(path.begin(), path.end(), '/', '\\');
  compile_node.append_attribute("Include") = path.c_str();
  AppendInlinedNodeChildPcData(compile_node, "SubType", "compile");
}

void cmAtmelStudio7TargetGenerator::BuildCompileItemGroup(pugi::xml_node& parent)
{
  // collect up group information
  std::vector<cmSourceGroup> sourceGroups = this->Makefile->GetSourceGroups();
  std::vector<cmGeneratorTarget::AllConfigSource> const& sources = this->GeneratorTarget->GetAllConfigSources();

  pugi::xml_node item_group_node = parent.append_child("ItemGroup");

  //
  for (cmGeneratorTarget::AllConfigSource const& si : sources) {
    const char* tool = nullptr;
    switch (si.Kind)
    {
      // Headers and sources both need to be present in the final project description
      // So that the files show up in Atmel Studio
      case cmGeneratorTarget::SourceKindObjectSource:
      {
        const std::string& lang = si.Source->GetLanguage();
        if (lang == "C" || lang == "CXX")
        {
          add_source_compile_node(item_group_node, si);
        }
      } break;

      case cmGeneratorTarget::SourceKindHeader: {
        const std::string& extension = si.Source->GetExtension();

        // Headers, for an unknown reason, are not recognized as C or CXX language files
        // So we need to check the extension instead (...)
        if (extension == "h")
        {
          add_source_compile_node(item_group_node, si);
        }
      } break;


      // NOTE : External Object source kind is not supported for now, investigations needed
      case cmGeneratorTarget::SourceKindExternalObject:
      default:
        tool = "None";
        break;
    }
  }
}

void cmAtmelStudio7TargetGenerator::BuildDevicePropertyGroup(pugi::xml_node& parent, const std::string& target_name, const std::string& lang)
{
  // @see http://www.gerald-fahrnholz.eu/sw/DocGenerated/HowToUse/html/group___grp_pugi_xml.html#pugi_xml_generated_file
  pugi::xml_node property_group = parent.append_child("PropertyGroup");
  AppendInlinedNodeChildPcData(property_group, "SchemaVersion", "2.0");
  AppendInlinedNodeChildPcData(property_group, "ProjectVersion", "7.0");

  // Preparse flags from the first configuration to retrieve Device's name !
  std::string device_name;
  {
    // Get languages
    std::vector<std::string> enabledLanguages;
    this->GlobalGenerator->GetEnabledLanguages(enabledLanguages);

    if (!enabledLanguages.empty()) {
      std::unordered_map<std::string, std::vector<std::string>> first_config_flags = RetrieveCmakeFlags(enabledLanguages, cmSystemTools::UpperCase(this->Configurations[0]));
      for (auto& flags : first_config_flags) {
        translator.parse(flags.second, flags.first);
      }

      // extract -mmcu option
      compiler::AbstractCompilerModel* comp = translator.get_compiler(enabledLanguages[0]);
      if (comp != nullptr) {
        compiler::CompilerOption* mmcu_opt = comp->get_option("-mmcu");
        if (mmcu_opt != nullptr) {
          compiler::MachineOption* opt = static_cast<compiler::MachineOption*>(mmcu_opt);
          device_name = AS7DeviceResolver::resolve_from_mmcu(opt->value);
          TargetedDevice.mmcu_option = opt->value;
        }

        // Try with definitions ... !
        if (device_name.empty()) {
          std::vector<std::string> options_vec = comp->get_all_options(compiler::CompilerOption::Type::Definition);
          device_name = AS7DeviceResolver::resolve_from_defines(options_vec);
        }
      }
    }
  }

  // If device name is still empty, it might be because we are in TryCompile mode and we need to provide a device for AS7 to build something
  if (device_name.empty()) {
    device_name = "ATmega328P";
  }

  // Update targeted Device member for further use in subsequent calls to other methods (for instance when building configurations xml...)
  AS7DeviceResolver::Core core = AS7DeviceResolver::resolve_core_from_name(device_name);
  TargetedDevice.DFP_name = AS7DeviceResolver::resolve_device_dfp_name(device_name);
  TargetedDevice.name = device_name;
  TargetedDevice.resolve_version(this->GlobalGenerator->GetAtmelStudio7InstallationFolder() + "\\packs");

  // TODO : put this elsewhere, this could easily be moved to the AS7DeviceResolver namespace or even in AS7Toolchains !
  // This could be a simple function such as :
  // std::string toolchain = AS7DeviceResolver::resolve_toolchain_name(core);
  std::string toolchain;
  switch (core) {

    case AS7DeviceResolver::Core::AT32UC:
      toolchain = "AVRGCC32";
      break;

    case AS7DeviceResolver::Core::AT90mega:
    case AS7DeviceResolver::Core::ATmega:
    case AS7DeviceResolver::Core::ATxmega:
    case AS7DeviceResolver::Core::ATtiny:
    // Default toolchain is set to AVRGCC8 to help with the TryCompile() step (AS7 needs a toolchain and a device)
    default:
      toolchain = "AVRGCC8";
      break;

    case AS7DeviceResolver::Core::ATSAM:
      toolchain = "ARMGCC";
      break;
  }

  // handles devices with mmcu option
  // TODO : this kind of functionality could be implemented in the AvrGCC8Common structure or whatnot.
  if (!TargetedDevice.mmcu_option.empty()) {
    translator.toolchain.common.Device = "-mmcu=" + TargetedDevice.mmcu_option;
    translator.toolchain.common.Device += " -B \"%24(PackRepoDir)\\atmel\\" + TargetedDevice.DFP_name + '\\' + TargetedDevice.version + "\\gcc\\dev\\" + TargetedDevice.mmcu_option + "\"";
  }

  AppendInlinedNodeChildPcData(property_group, "ToolchainName", "com.Atmel." + toolchain + "." + lang);
  AppendInlinedNodeChildPcData(property_group, "ProjectGuid", this->GUID);
  AppendInlinedNodeChildPcData(property_group, "avrdevice", device_name);
  AppendInlinedNodeChildPcData(property_group, "avrdeviceseries", "none");

  // Shared libraries are not supported for now as they imply to use some real time OS (?)
  if (this->GeneratorTarget->GetType() == cmStateEnums::TargetType::EXECUTABLE) {
    AppendInlinedNodeChildPcData(property_group, "OutputType", "Executable");
  } else {
    AppendInlinedNodeChildPcData(property_group, "OutputType", "StaticLibrary");
  }

  AppendInlinedNodeChildPcData(property_group, "Language", lang);

  if (this->GeneratorTarget->GetType() == cmStateEnums::TargetType::EXECUTABLE) {
    AppendInlinedNodeChildPcData(property_group, "OutputFileName", "$(MSBuildProjectName)");
    AppendInlinedNodeChildPcData(property_group, "OutputFileExtension", ".elf");
  } else {
    AppendInlinedNodeChildPcData(property_group, "OutputFileName", "lib$(MSBuildProjectName)");
    AppendInlinedNodeChildPcData(property_group, "OutputFileExtension", ".a");
  }
  AppendInlinedNodeChildPcData(property_group, "OutputDirectory", "$(MSBuildProjectDirectory)\\$(Configuration)");
  AppendInlinedNodeChildPcData(property_group, "AssemblyName", target_name);
  AppendInlinedNodeChildPcData(property_group, "Name", target_name);
  AppendInlinedNodeChildPcData(property_group, "RootNamespace", target_name);
  AppendInlinedNodeChildPcData(property_group, "ToolchainFlavour", "Native");
  AppendInlinedNodeChildPcData(property_group, "KeepTimersRunning", "true");
  AppendInlinedNodeChildPcData(property_group, "OverrideVtor", "false");
  AppendInlinedNodeChildPcData(property_group, "CacheFlash", "true");
  AppendInlinedNodeChildPcData(property_group, "ProgFlashFromRam", "true");
  AppendInlinedNodeChildPcData(property_group, "RamSnippetAddress", "0x20000000");
  AppendInlinedNodeChildPcData(property_group, "UncachedRange");
  AppendInlinedNodeChildPcData(property_group, "preserveEEPROM", "true");
  AppendInlinedNodeChildPcData(property_group, "OverrideVtorValue", "exception_table");
  AppendInlinedNodeChildPcData(property_group, "BootSegment", "2");
  AppendInlinedNodeChildPcData(property_group, "ResetRule", "0");
  AppendInlinedNodeChildPcData(property_group, "eraseonlaunchrule", "0");
  AppendInlinedNodeChildPcData(property_group, "EraseKey");

  pugi::xml_node asf_framework_config = property_group.append_child("AsfFrameworkConfig");
  pugi::xml_node framework_data_node = asf_framework_config.append_child("framework-data");
  framework_data_node.append_attribute("xmlns").set_value("");
  AppendInlinedNodeChildPcData(framework_data_node, "options");
  AppendInlinedNodeChildPcData(framework_data_node, "configurations");
  AppendInlinedNodeChildPcData(framework_data_node, "files");

  pugi::xml_node documentation_node = framework_data_node.append_child("documentation");
  documentation_node.append_attribute("help").set_value("");
  pugi::xml_node offline_documentation_node = framework_data_node.append_child("offline-documentation");
  offline_documentation_node.append_attribute("help").set_value("");

  pugi::xml_node dependencies_node = framework_data_node.append_child("dependencies");
  pugi::xml_node content_extension_node = dependencies_node.append_child("content-extension");
  content_extension_node.append_attribute("eid").set_value("atmel.asf");
  content_extension_node.append_attribute("uuidref").set_value("Atmel.ASF");
  content_extension_node.append_attribute("version").set_value("3.49.1");

  BuildSimulatorConfiguration(property_group);
}

// NOTE : this implementation of Simulator configuration is a default one and does not reflect any expected configuration
void cmAtmelStudio7TargetGenerator::BuildSimulatorConfiguration(pugi::xml_node& parent, const std::string& device_signature, const std::string& stimuli_filepath)
{
  AppendInlinedNodeChildPcData(parent, "avrtool", "com.atmel.avrdbg.tool.simulator");
  AppendInlinedNodeChildPcData(parent, "avrtoolserialnumber");
  AppendInlinedNodeChildPcData(parent, "avrdeviceexpectedsignature", device_signature.c_str());
  pugi::xml_node simulator_configuration_node = parent.append_child("com_atmel_avrdbg_tool_simulator");
  pugi::xml_node tool_options_node = simulator_configuration_node.append_child("ToolOptions");
  AppendInlinedNodeChildPcData(tool_options_node, "InterfaceProperties");
  AppendInlinedNodeChildPcData(tool_options_node, "InterfaceName");

  AppendInlinedNodeChildPcData(simulator_configuration_node, "ToolType", "com.atmel.avrdbg.tool.simulator");
  AppendInlinedNodeChildPcData(simulator_configuration_node, "ToolNumber");
  AppendInlinedNodeChildPcData(simulator_configuration_node, "ToolName", "Simulator");

  if (!stimuli_filepath.empty()) {
    AppendInlinedNodeChildPcData(parent, "StimuliFile", stimuli_filepath.c_str());
  }
  AppendInlinedNodeChildPcData(parent, "avrtoolinterface");
}

void cmAtmelStudio7TargetGenerator::BuildProjectReferenceItemGroup(pugi::xml_node& parent)
{
  cmGlobalVisualStudioGenerator::OrderedTargetDependSet target_dependencies = GetTargetDependencies();

  auto item_group_node = parent.append_child("ItemGroup");

  for (cmGeneratorTarget const* dependent_target : target_dependencies) {
    if (!dependent_target->IsInBuildSystem()) {
      continue;
    }

    cmLocalGenerator* lg = dependent_target->GetLocalGenerator();
    std::string name = dependent_target->GetName();
    std::string path;
    cmProp p = dependent_target->GetProperty("EXTERNAL_MSPROJECT");
    if (p != nullptr) {
      path = *p;
    } else {
      path = cmStrCat(lg->GetCurrentBinaryDirectory(), '/', dependent_target->GetName(),
                      AS7ProjectDescriptor::get_extension(computeProjectFileExtension(dependent_target)));
    }

    // Convert to windows paths
    path = cmutils::strings::replace(path, "/", "\\");

    auto project_reference_node = item_group_node.append_child("ProjectReference");
    project_reference_node.append_attribute("Include") = path.c_str();
    AppendInlinedNodeChildPcData(project_reference_node, "Name", name);
    AppendInlinedNodeChildPcData(project_reference_node, "Project", "{" + this->GlobalGenerator->GetGUID(name) + "}");
    AppendInlinedNodeChildPcData(project_reference_node, "Private", "True");
  }
}

cmGlobalVisualStudioGenerator::OrderedTargetDependSet cmAtmelStudio7TargetGenerator::GetTargetDependencies() const
{
  cmGlobalGenerator::TargetDependSet const& unordered = this->GlobalGenerator->GetTargetDirectDepends(this->GeneratorTarget);
  cmGlobalVisualStudioGenerator::OrderedTargetDependSet depends(unordered, CMAKE_CHECK_BUILD_SYSTEM_TARGET);
  return depends;
}

std::array<AS7ProjectDescriptor::Properties,
           static_cast<uint8_t>(AS7ProjectDescriptor::Type::maxval)>
  AS7ProjectDescriptor::collection = {
    Properties("cppproj", Type::cppproj, "CXX", "$(AVRSTUDIO_EXE_PATH)\\Vs\\Compiler.targets"),
    Properties("cproj", Type::cproj, "C", "$(AVRSTUDIO_EXE_PATH)\\Vs\\Compiler.targets"),
    Properties("asmproj", Type::asmproj, "ASM", "$(AVRSTUDIO_EXE_PATH)\\Vs\\Compiler.targets")
  };

AS7ProjectDescriptor::Properties* AS7ProjectDescriptor::get_project_type_properties(const std::string& name)
{
  auto item = std::find_if(collection.begin(), collection.end(), [name](const Properties& prop) {
    return name == prop.name;
  });

  if (item != collection.end()) {
    return &(*item);
  }

  return nullptr;
}

AS7ProjectDescriptor::Properties* AS7ProjectDescriptor::get_project_type_properties(const AS7ProjectDescriptor::Type type)
{
  auto item = std::find_if(collection.begin(), collection.end(), [type](const Properties& prop) {
    return type == prop.type;
  });

  if (item != collection.end()) {
    return &(*item);
  }

  return nullptr;
}

std::string AS7ProjectDescriptor::get_extension(Type type)
{
  auto prop = get_project_type_properties(type);
  if (nullptr != prop) {
    return "." + prop->name;
  }

  return "";
}

std::string cmAtmelStudio7TargetGenerator::TargetedDevice_t::get_dfp_path(const std::string& packs_path) const
{
  return packs_path + "\\atmel\\" + DFP_name;
}

std::string cmAtmelStudio7TargetGenerator::TargetedDevice_t::resolve_version(const std::string& packs_path)
{
  std::string dfp_dir = get_dfp_path(packs_path);
  version = AS7DeviceResolver::get_max_packs_version(dfp_dir);
  return version;
}

std::string cmAtmelStudio7TargetGenerator::TargetedDevice_t::get_dfp_include_dir(const std::string& packs_path)
{
  if (version.empty()) {
    version = resolve_version(packs_path);
  }
  return "%24(PackRepoDir)\\atmel\\" + DFP_name + "\\" + version + "\\include\\";
}
