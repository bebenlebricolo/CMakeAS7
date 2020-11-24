/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmAtmelStudio7TargetGenerator.h"

#include <algorithm>
#include <iterator>
#include <set>

#include <cm/memory>
#include <cm/string_view>
#include <cm/vector>
#include <cmext/algorithm>

#include "cmGeneratedFileStream.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalAtmelStudio7Generator.h"
#include "cmGlobalVisualStudioVersionedGenerator.h"
#include "cmLinkLineDeviceComputer.h"
#include "cmLocalAtmelStudio7Generator.h"
#include "cmMakefile.h"
#include "cmSourceFile.h"
#include "cmStringAlgorithms.h"
#include "cmStringUtils.h"
#include "cmSystemTools.h"

#include "AS7ToolchainTranslator.h"
#include "pugixml.hpp"

cmAtmelStudio7TargetGenerator::cmAtmelStudio7TargetGenerator(
  cmGeneratorTarget* target, cmGlobalAtmelStudio7Generator* gg)
  : GeneratorTarget(target)
  , Makefile(target->Target->GetMakefile())
  , Platform(gg->GetPlatform(gg->GetCurrentPlatform()))
  , Name(target->GetName())
  , GUID(gg->GetGUID(this->Name))
  , GlobalGenerator(gg)
  , LocalGenerator((cmLocalAtmelStudio7Generator*)target->GetLocalGenerator())
{
  this->Configurations =
    this->Makefile->GetGeneratorConfigs(cmMakefile::ExcludeEmptyConfig);
  //this->LocalGenerator->GetCurrentBinaryDirectory() + "/" +
  //  this->LocalGenerator->GetTargetDirectory(this->GeneratorTarget);
  this->InSourceBuild = (this->Makefile->GetCurrentSourceDirectory() ==
                         this->Makefile->GetCurrentBinaryDirectory());
  //this->ClassifyAllConfigSources();
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

  // Defaults to c projects
  return AS7ProjectDescriptor::Type::cproj;
}

void cmAtmelStudio7TargetGenerator::Generate()
{
  // Retrieve project file extension
  const AS7ProjectDescriptor::Type ProjectFileType =
    computeProjectFileExtension(this->GeneratorTarget);
  const std::string ProjectFileExtension = AS7ProjectDescriptor::get_extension(ProjectFileType);

  // Tell the global generator the name of the project file
  this->GeneratorTarget->Target->SetProperty("GENERATOR_FILE_NAME",
                                             this->Name);
  this->GeneratorTarget->Target->SetProperty("GENERATOR_FILE_NAME_EXT",
                                             ProjectFileExtension);
  this->AdditionalUsingDirectories.clear();

  std::string path =
    cmStrCat(this->LocalGenerator->GetCurrentBinaryDirectory(), '/',
             this->Name, ProjectFileExtension);
  cmGeneratedFileStream BuildFileStream(path);
  const std::string PathToProjectFile = path;
  BuildFileStream.SetCopyIfDifferent(true);

  pugi::xml_document doc;
  // Prepend delcaration node will look like this : <?xml version="1.0" encoding="utf-8"?>
  pugi::xml_node decl = doc.prepend_child(pugi::node_declaration);
  decl.append_attribute("version") = "1.0";
  decl.append_attribute("encoding") = "utf-8";

  pugi::xml_node project_node = doc.append_child(pugi::node_element);
  project_node.set_name("Project");
  project_node.append_attribute("DefaultTargets") = "Build";
  project_node.append_attribute("xmlns") = "http://schemas.microsoft.com/developer/msbuild/2003";
  project_node.append_attribute("ToolsVersion") = "14.0";

  BuildDevicePropertyGroup(project_node, this->Name);

  std::vector<cmGeneratorTarget::AllConfigSource> const& sources =
    this->GeneratorTarget->GetAllConfigSources();

  for (std::string const& config : this->Configurations) {
    std::string configUpper = cmSystemTools::UpperCase(config);
    BuildConfigurationXmlGroup(project_node, config);
  }

  // Compile item group which lists sources to be built as part of this target
  BuildCompileItemGroup(project_node);

  // Add projects references
  BuildProjectReferenceItemGroup(project_node);

  // Last node is dedicated to atmelstudio specific targets
  pugi::xml_node import_projects_node = project_node.append_child("Import");
  import_projects_node.append_attribute("Project") = R"($(AVRSTUDIO_EXE_PATH)\\Vs\\Compiler.targets)";

  //doc.save_file(PathToProjectFile.c_str(), "  ", (pugi::format_indent | pugi::format_write_bom), pugi::xml_encoding::encoding_utf8);
  doc.save(BuildFileStream, "  ", (pugi::format_indent | pugi::format_write_bom), pugi::xml_encoding::encoding_utf8);
  BuildFileStream.Close();
}

void cmAtmelStudio7TargetGenerator::WriteTargetGlobalProperties(pugi::xml_node& node)
{
}

//void cmAtmelStudio7TargetGenerator::ClassifyAllConfigSources()
//{
//    for (cmGeneratorTarget::AllConfigSource const& source :
//         this->GeneratorTarget->GetAllConfigSources()) {
//      this->ClassifyAllConfigSource(source);
//    }
//}

//void cmAtmelStudio7TargetGenerator::ClassifyAllConfigSource(cmGeneratorTarget::AllConfigSource const& acs)
//{
//  switch (acs.Kind) {
//    case cmGeneratorTarget::SourceKindResx: {
//      // Build and save the name of the corresponding .h file
//      // This relationship will be used later when building the project files.
//      // Both names would have been auto generated from Visual Studio
//      // where the user supplied the file name and Visual Studio
//      // appended the suffix.
//      std::string resx = acs.Source->ResolveFullPath();
//      std::string hFileName = resx.substr(0, resx.find_last_of('.')) + ".h";
//      this->ExpectedResxHeaders.insert(hFileName);
//    } break;
//    case cmGeneratorTarget::SourceKindXaml: {
//      // Build and save the name of the corresponding .h and .cpp file
//      // This relationship will be used later when building the project files.
//      // Both names would have been auto generated from Visual Studio
//      // where the user supplied the file name and Visual Studio
//      // appended the suffix.
//      std::string xaml = acs.Source->ResolveFullPath();
//      std::string hFileName = xaml + ".h";
//      std::string cppFileName = xaml + ".cpp";
//      this->ExpectedXamlHeaders.insert(hFileName);
//      this->ExpectedXamlSources.insert(cppFileName);
//    } break;
//    default:
//      break;
//  }
//}

void cmAtmelStudio7TargetGenerator::AppendInlinedNodeChildPcData(pugi::xml_node& parent, const std::string& node_name, const std::string& value)
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

std::vector<std::string> cmAtmelStudio7TargetGenerator::ConvertStringRange(const cmStringRange& range) const
{
  std::vector<std::string> out;
  for (auto& val : range) {
    out.push_back(val);
  }
  return out;
}

std::unordered_map<std::string, std::vector<std::string>> cmAtmelStudio7TargetGenerator::RetrieveCmakeFlags(const std::vector<std::string>& languages,
                                                                                                            const std::string& upConfig)
{
  std::unordered_map<std::string, std::vector<std::string>> out;
  std::string linker_variable_name = "CMAKE_EXE_LINKER_FLAGS";

  for (auto& lang : languages) {
    std::vector<std::string> all_flags;
    std::string flag_variable_name = "CMAKE_" + lang + "_FLAGS";
    std::string flag_variable_config_name = flag_variable_name + "_" + upConfig;

    cmProp lang_flags_base = this->Makefile->GetDefinition(flag_variable_name);
    cmProp lang_flags_config = this->Makefile->GetDefinition(flag_variable_config_name);
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
  AvrToolchain::AS7ToolchainTranslator translator;

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

  if (build_type != "Release") {
    translator.toolchain.avrgcc.symbols.def_symbols.push_back("NDEBUG");
    translator.toolchain.avrgcccpp.symbols.def_symbols.push_back("NDEBUG");
  }

  // Handle include paths
  // TODO : use parsed device representation to compute the right DFP for this device. Default for now
  translator.toolchain.avrgcc.directories.include_paths.push_back("%24(PackRepoDir)\\atmel\\ATmega_DFP\\1.2.209\\include");
  translator.toolchain.avrgcccpp.directories.include_paths.push_back("%24(PackRepoDir)\\atmel\\ATmega_DFP\\1.2.209\\include");
  for (const auto& lang : enabledLanguages) {
    auto includesList = this->GetIncludes(build_type, lang);
    for (const auto& singleInclude : includesList) {
      if (lang == "CXX") {
        translator.toolchain.avrgcccpp.directories.include_paths.push_back(singleInclude);
      } else if (lang == "C") {
        translator.toolchain.avrgcc.directories.include_paths.push_back(singleInclude);
      }
    }
  }

  // Configure linker
  translator.toolchain.linker.libraries.libraries.push_back("libm");
  cmGlobalVisualStudioGenerator::OrderedTargetDependSet target_dependencies = GetTargetDependencies();

  for (cmGeneratorTarget const* dependent_target : target_dependencies) {
    if (!dependent_target->IsInBuildSystem()) {
      continue;
    }
    cmLocalGenerator* lg = dependent_target->GetLocalGenerator();
    translator.toolchain.linker.libraries.libraries.push_back(dependent_target->GetName());
    if (lg != nullptr) {
      std::string build_directory = lg->GetCurrentBinaryDirectory() + "/" + build_type;
      translator.toolchain.linker.libraries.search_path.push_back(cmutils::strings::replace(build_directory, "/", "\\"));
    }
  }
  translator.toolchain.assembler.general.include_path.push_back("%24(PackRepoDir)\\atmel\\ATmega_DFP\\1.2.209\\include");

  translator.generate_xml(avr_gcc_node);
}

void cmAtmelStudio7TargetGenerator::BuildCompileItemGroup(pugi::xml_node& parent)
{
  // collect up group information
  std::vector<cmSourceGroup> sourceGroups = this->Makefile->GetSourceGroups();
  // TODO : add source files

  std::vector<cmGeneratorTarget::AllConfigSource> const& sources =
    this->GeneratorTarget->GetAllConfigSources();

  pugi::xml_node item_group_node = parent.append_child("ItemGroup");

  for (cmGeneratorTarget::AllConfigSource const& si : sources) {
    const char* tool = nullptr;
    switch (si.Kind) {
      case cmGeneratorTarget::SourceKindObjectSource: {
        const std::string& lang = si.Source->GetLanguage();
        if (lang == "C" || lang == "CXX") {
          tool = "Compile";
          pugi::xml_node compile_node = item_group_node.append_child(tool);
          std::string path = si.Source->GetFullPath().c_str();
          // Convert regular slashes to Windows backslashes
          std::replace(path.begin(), path.end(), '/', '\\');
          compile_node.append_attribute("Include") = path.c_str();
          AppendInlinedNodeChildPcData(compile_node, "SubType", "compile");
        }
      }
      break;

      case cmGeneratorTarget::SourceKindExternalObject:
        tool = "Object";
        if (this->LocalGenerator) {
          std::vector<cmSourceFile*> const* d =
            this->GeneratorTarget->GetSourceDepends(si.Source);
          if (d && !d->empty()) {
            tool = "None";
          }
        }
        break;

      default:
        tool = "None";
        break;
    }
  }
}

void cmAtmelStudio7TargetGenerator::BuildDevicePropertyGroup(pugi::xml_node& parent, const std::string& target_name)
{
  // @see http://www.gerald-fahrnholz.eu/sw/DocGenerated/HowToUse/html/group___grp_pugi_xml.html#pugi_xml_generated_file
  pugi::xml_node property_group = parent.append_child("PropertyGroup");
  AppendInlinedNodeChildPcData(property_group, "SchemaVersion", "2.0");
  AppendInlinedNodeChildPcData(property_group, "ProjectVersion", "7.0");
  AppendInlinedNodeChildPcData(property_group, "ToolchainName", "com.Atmel.AVRGCC8.C");
  AppendInlinedNodeChildPcData(property_group, "ProjectGuid", this->GUID);
  AppendInlinedNodeChildPcData(property_group, "avrdevice", "ATmega328P");
  AppendInlinedNodeChildPcData(property_group, "avrdeviceseries", "none");

  if (this->GeneratorTarget->GetType() == cmStateEnums::TargetType::EXECUTABLE) {
    AppendInlinedNodeChildPcData(property_group, "OutputType", "Executable");
  } else {
    AppendInlinedNodeChildPcData(property_group, "OutputType", "StaticLibrary");
  }

  AppendInlinedNodeChildPcData(property_group, "Language", "C");

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
  cmGlobalVisualStudioGenerator::OrderedTargetDependSet& target_dependencies = GetTargetDependencies();

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
  cmGlobalGenerator::TargetDependSet const& unordered =
    this->GlobalGenerator->GetTargetDirectDepends(this->GeneratorTarget);
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