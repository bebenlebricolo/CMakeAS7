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
#include "cmSystemTools.h"

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
  this->ClassifyAllConfigSources();
}

cmAtmelStudio7TargetGenerator::~cmAtmelStudio7TargetGenerator()
{
}

static AS7ProjectDescriptor::Type computeProjectFileExtension(cmGeneratorTarget const* t)
{
  AS7ProjectDescriptor::Type returnedType;
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

  BuildDevicePropertyGroup(project_node, "GenericApplication");
  BuildConfigurationXmlGroup(project_node, "Release");
  BuildConfigurationXmlGroup(project_node, "Debug");

  // Compile item group which lists sources to be built as part of this target
  BuildCompileItemGroup(project_node);

  // Last node is dedicated to atmelstudio specific targets
  pugi::xml_node import_projects_node = project_node.append_child("Import");
  import_projects_node.append_attribute("Project") = R"($(AVRSTUDIO_EXE_PATH)\\Vs\\Compiler.targets)";

  doc.save_file(PathToProjectFile.c_str(), "  ", (pugi::format_indent | pugi::format_write_bom), pugi::xml_encoding::encoding_utf8);
}

void cmAtmelStudio7TargetGenerator::WriteTargetGlobalProperties(pugi::xml_node& node)
{
}

void cmAtmelStudio7TargetGenerator::AppendInlinedNodeChildPcData(pugi::xml_node& parent, const std::string& node_name, const std::string& value)
{
  pugi::xml_node node = parent.append_child(node_name.c_str());
  if (0 != value.size()) {
    pugi::xml_node child = node.append_child(pugi::node_pcdata);
    child.set_value(value.c_str());
  }
}

void cmAtmelStudio7TargetGenerator::BuildConfigurationXmlGroup(pugi::xml_node& parent, const std::string& build_type)
{
  pugi::xml_node property_group_node = parent.append_child("PropertyGroup");
  std::string conditionnal_str = " '$(Configuration)' == '" + build_type + "' ";
  property_group_node.append_attribute("Condition").set_value(conditionnal_str.c_str());

  pugi::xml_node toolchain_settings_node = property_group_node.append_child("ToolchainSettings");
  pugi::xml_node avr_gcc_node = toolchain_settings_node.append_child("AvrGcc");

  AppendInlinedNodeChildPcData(avr_gcc_node, "avrgcc.common.Device", "-mmcu=atmega328p -B \"%24(PackRepoDir)\\atmel\\ATmega_DFP\\1.2.209\\gcc\\dev\\atmega328p\"");
  AppendInlinedNodeChildPcData(avr_gcc_node, "avrgcc.common.outputfiles.hex", "True");
  AppendInlinedNodeChildPcData(avr_gcc_node, "avrgcc.common.outputfiles.lss", "True");
  AppendInlinedNodeChildPcData(avr_gcc_node, "avrgcc.common.outputfiles.eep", "True");
  AppendInlinedNodeChildPcData(avr_gcc_node, "avrgcc.common.outputfiles.srec", "True");
  AppendInlinedNodeChildPcData(avr_gcc_node, "avrgcc.common.outputfiles.usersignatures", "False");
  AppendInlinedNodeChildPcData(avr_gcc_node, "avrgcc.compiler.general.ChangeDefaultCharTypeUnsigned", "True");
  AppendInlinedNodeChildPcData(avr_gcc_node, "avrgcc.compiler.general.ChangeDefaultBitFieldUnsigned", "True");

  // Preprocessor definitions
  pugi::xml_node avr_gcc_defsymbols_node = avr_gcc_node.append_child("avrgcc.compiler.symbols.DefSymbols");
  pugi::xml_node list_values_node = avr_gcc_defsymbols_node.append_child("ListValues");
  AppendInlinedNodeChildPcData(list_values_node, "Value", "NDEBUG");

  // include directories
  pugi::xml_node include_directories_node = avr_gcc_node.append_child("avrgcc.compiler.directories.IncludePaths");
  list_values_node = include_directories_node.append_child("ListValues");
  AppendInlinedNodeChildPcData(list_values_node, "Value", "%24(PackRepoDir)\\atmel\\ATmega_DFP\\1.2.209\\include");

  // Optimizations
  AppendInlinedNodeChildPcData(avr_gcc_node, "avrgcc.compiler.optimization.level", "Optimize for size (-Os)");
  AppendInlinedNodeChildPcData(avr_gcc_node, "avrgcc.compiler.optimization.PackStructureMembers", "True");
  AppendInlinedNodeChildPcData(avr_gcc_node, "avrgcc.compiler.optimization.AllocateBytesNeededForEnum", "True");
  AppendInlinedNodeChildPcData(avr_gcc_node, "avrgcc.compiler.warnings.AllWarnings", "True");

  // Configure linker link libraries
  pugi::xml_node linker_libraries_node = avr_gcc_node.append_child("avrgcc.linker.libraries.Libraries");
  list_values_node = linker_libraries_node.append_child("ListValues");
  AppendInlinedNodeChildPcData(list_values_node, "Value", "libm");

  // Assembler include directories
  pugi::xml_node assembler_includes_node = avr_gcc_node.append_child("avrgcc.assembler.general.IncludePaths");
  list_values_node = assembler_includes_node.append_child("ListValues");
  AppendInlinedNodeChildPcData(list_values_node, "Value", "%24(PackRepoDir)\\atmel\\ATmega_DFP\\1.2.209\\include");
}

void cmAtmelStudio7TargetGenerator::BuildCompileItemGroup(pugi::xml_node& parent)
{
  pugi::xml_node item_group_node = parent.append_child("ItemGroup");
  pugi::xml_node compile_node = item_group_node.append_child("Compile");
  compile_node.append_attribute("Include") = "main.c";
  AppendInlinedNodeChildPcData(compile_node, "SubType", "compile");
}

void cmAtmelStudio7TargetGenerator::BuildDevicePropertyGroup(pugi::xml_node& parent, const std::string& target_name)
{
  // @see http://www.gerald-fahrnholz.eu/sw/DocGenerated/HowToUse/html/group___grp_pugi_xml.html#pugi_xml_generated_file
  pugi::xml_node property_group = parent.append_child("PropertyGroup");
  AppendInlinedNodeChildPcData(property_group, "SchemaVersion", "2.0");
  AppendInlinedNodeChildPcData(property_group, "ProjectVersion", "7.0");
  AppendInlinedNodeChildPcData(property_group, "ToolchainName", "com.Atmel.AVRGCC8.C");
  AppendInlinedNodeChildPcData(property_group, "ProjectGuid", "dce6c7e3-ee26-4d79-826b-08594b9ad897");
  AppendInlinedNodeChildPcData(property_group, "avrdevice", "ATmega328P");
  AppendInlinedNodeChildPcData(property_group, "avrdeviceseries", "none");
  AppendInlinedNodeChildPcData(property_group, "OutputType", "Executable");
  AppendInlinedNodeChildPcData(property_group, "Language", "C");
  AppendInlinedNodeChildPcData(property_group, "OutputFileName", "$(MSBuildProjectName)");
  AppendInlinedNodeChildPcData(property_group, "OutputFileExtension", ".elf");
  AppendInlinedNodeChildPcData(property_group, "OutputDirectory", "$(MSBuildProjectDirectory)\\$(Configuration)");
  AppendInlinedNodeChildPcData(property_group, "AssemblyName", target_name);
  AppendInlinedNodeChildPcData(property_group, "Name", target_name);
  AppendInlinedNodeChildPcData(property_group, "RootNamespace", target_name);
  AppendInlinedNodeChildPcData(property_group, "ToolchainFlavour", "Native");
  AppendInlinedNodeChildPcData(property_group, "KeepTimersRunning", "true");
  AppendInlinedNodeChildPcData(property_group, "OverrideVtor", "false");
  AppendInlinedNodeChildPcData(property_group, "CacheFlash", "true");
  AppendInlinedNodeChildPcData(property_group, "ProgFlashFromRam", "true");
  AppendInlinedNodeChildPcData(property_group, "RamSnippetAddress", "0x2000000");
  AppendInlinedNodeChildPcData(property_group, "UncachedRange");
  AppendInlinedNodeChildPcData(property_group, "preserveEEPROM", "true");
  AppendInlinedNodeChildPcData(property_group, "OverrideVtorValue");
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
  content_extension_node.append_attribute("version").set_value("3.40.0");

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