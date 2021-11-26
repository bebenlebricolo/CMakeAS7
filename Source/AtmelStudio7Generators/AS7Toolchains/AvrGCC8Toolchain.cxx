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

#include "AvrGCC8Toolchain.h"

#include <algorithm>

#include "cmAvrGccCompiler.h"
#include "cmAvrGccCompilerOption.h"
#include "cmAvrGccDebugOption.h"
#include "cmAvrGccDefinitionOption.h"
#include "cmAvrGccLanguageStandardOption.h"
#include "cmAvrGccMachineOption.h"
#include "cmAvrGccOptimizationOption.h"
#include "cmStringUtils.h"

#include "pugixml.hpp"

namespace AvrToolchain {

void AS7AvrGCC8::clear()
{
  common.clear();
  avrgcc.clear();
  avrgcccpp.clear();
  linker.clear();
  assembler.clear();
  archiver_flags.clear();
}

void AS7AvrGcc8_Base::clear()
{
  general.subroutine_function_prologue = false;
  general.change_stack_pointer_without_disabling_interrupt = false;
  general.change_default_bitfield_unsigned = true;
  general.change_default_chartype_unsigned = true;

  preprocessor.do_not_search_system_directories = false;
  preprocessor.preprocess_only = false;

  symbols.def_symbols.clear();
  directories.include_paths.clear();

  optimizations.level.clear();
  optimizations.other_flags.clear();
  optimizations.prepare_data_for_garbage_collection = true;
  optimizations.prepare_function_for_garbage_collection = true;
  optimizations.pack_structure_members = true;
  optimizations.allocate_bytes_needed_for_enum = true;
  optimizations.use_short_calls = false;
  optimizations.debug_level.clear();
  optimizations.other_debugging_flags.clear();

  warnings.all_warnings = true;
  warnings.extra_warnings = true;
  warnings.undefined = false;
  warnings.warnings_as_error = false;
  warnings.check_syntax_only = false;
  warnings.pedantic = false;
  warnings.pedantic_warnings_as_errors = false;
  warnings.inhibit_all_warnings = false;

  miscellaneous.other_flags.clear();
  miscellaneous.verbose = false;
  miscellaneous.support_ansi_programs = false;
  miscellaneous.do_not_delete_temporary_files = false;
}

void AS7AvrGCC8Linker::clear()
{
  general.do_not_use_default_libraries = false;
  general.do_not_use_standard_start_file = false;
  general.no_startup_or_default_libs = false;
  general.omit_all_symbol_information = false;
  general.no_shared_libraries = true;
  general.generate_map_file = true;
  general.use_vprintf_library = false;

  libraries.libraries.clear();
  libraries.search_path.clear();

  optimizations.garbage_collect_unused_sections = true;
  optimizations.put_read_only_data_in_writable_data_section = false;

  miscellaneous.linker_flags.clear();
}

void AS7AvrGCC8Assembler::clear()
{
  general.include_path.clear();
  general.anounce_version = false;
  debugging.debug_level.clear();
}

AS7AvrGcc8_Base::AS7AvrGcc8_Base(const std::string& lang_standard)
{
  // Make room for subsequent flags
  miscellaneous.other_flags = lang_standard + " ";
}

AS7AvrGcc8_Base::AS7AvrGcc8_Base(const AS7AvrGcc8_Base& other)
{
  copy_from(other);
}

void AS7AvrGcc8_Base::copy_from(const AS7AvrGcc8_Base& other)
{
  general = other.general;
  preprocessor = other.preprocessor;
  symbols = other.symbols;
  directories.include_paths = other.directories.include_paths;
  optimizations = other.optimizations;
  warnings = other.warnings;
  miscellaneous = other.miscellaneous;
}

std::string AS7AvrGCC8::get_unsupported_options(const compiler::cmAvrGccCompiler& parser,
                                                const compiler::CompilerOption::Type type,
                                                const std::vector<std::string>& options) const
{
  std::string out;
  std::vector<std::string> unsupported_optim = parser.get_unsupported_options(type, options);
  for (uint8_t i = 0; i < unsupported_optim.size(); i++) {
    out += unsupported_optim[i];
    if (i != (unsupported_optim.size() - 1)) {
      out += " ";
    }
  }
  return out;
}

std::vector<std::string> AS7AvrGCC8::get_all_supported_options() const
{
  std::vector<std::string> out;
  const std::vector<std::string>& common_vec = common.get_supported_options();
  const std::vector<std::string>& avrgcc_vec = avrgcc.get_supported_options();
  const std::vector<std::string>& linker_vec = linker.get_supported_options();
  const std::vector<std::string>& assem_vec = assembler.get_supported_options();

  out.insert(out.end(), common_vec.begin(), common_vec.end());
  out.insert(out.end(), avrgcc_vec.begin(), avrgcc_vec.end());
  out.insert(out.end(), linker_vec.begin(), linker_vec.end());
  out.insert(out.end(), assem_vec.begin(), assem_vec.end());

  return out;
}

void AS7AvrGCC8::convert_from(const compiler::cmAvrGccCompiler& parser, const std::string& lang)
{
  //parser.get_options(compiler::CompilerOption::Type::Optimization)
  AS7AvrGcc8_Base* tool = nullptr;
  if (lang == "C") {
    tool = &avrgcc;
  } else {
    tool = &avrgcccpp;
  }

  // TODO : Use the device resolver to perform device deduction. note that the TargetedDevice code from TargetGenerator
  // first has to be moved closer for this to be enabled. For now, default -mmcu handling is proposed

  // This option is a bit special and requires dedicated handling
  if (common.Device.empty()) {
    if (parser.has_option("-mmcu")) {
      compiler::CompilerOption* opt = parser.get_option("-mmcu");
      auto option = dynamic_cast<compiler::MachineOption*>(opt);
      common.Device = "-mmcu=" + option->value;
    }
    common.Device += " -B \"%24(PackRepoDir)\\atmel\\ATmega_DFP\\1.2.209\\gcc\\dev\\atmega328p\"";
  }

  common.relax_branches = parser.has_option("-mrelax");

  tool->general.subroutine_function_prologue = parser.has_option("-mcall-prologues");
  tool->general.change_stack_pointer_without_disabling_interrupt = parser.has_option("-mno-interrupts");
  tool->general.change_default_chartype_unsigned = parser.has_option("-funsigned-char");
  tool->general.change_default_bitfield_unsigned = parser.has_option("-funsigned-bitfields");

  tool->preprocessor.do_not_search_system_directories = parser.has_option("-nostdinc");
  tool->preprocessor.preprocess_only = parser.has_option("-E");

  // Retrieve definitions symbols
  {
    const auto& definitions = parser.get_options(compiler::CompilerOption::Type::Definition);
    for (const auto& def : definitions) {
      auto* def_ptr = dynamic_cast<compiler::DefinitionOption*>(def.get());
      tool->symbols.def_symbols.push_back(def_ptr->defsymbol);
    }
  }

  // Extract optimizations
  {
    const auto& optimizations = parser.get_options(compiler::CompilerOption::Type::Optimization);
    const auto& max_opt = std::max_element(optimizations.begin(), optimizations.end());

    // Default optimization
    if (max_opt == optimizations.end()) {
      tool->optimizations.level = compiler::OptimizationOption::get_default().second.atmel_studio_description;
    } else {
      tool->optimizations.level = (*max_opt)->generate(true);
    }
  }

  tool->optimizations.prepare_function_for_garbage_collection = parser.has_option("-ffunction-sections");
  tool->optimizations.prepare_data_for_garbage_collection = parser.has_option("-fdata-sections");
  tool->optimizations.pack_structure_members = parser.has_option("-fpack-struct");
  tool->optimizations.allocate_bytes_needed_for_enum = parser.has_option("-fshort-enums");
  tool->optimizations.use_short_calls = parser.has_option("-mshort-calls");

  // List "other" optimizations flags (a.k.a unsupported flags)
  tool->optimizations.other_flags = get_unsupported_options(parser,
                                                            compiler::CompilerOption::Type::Optimization,
                                                            tool->get_supported_optimizations_options());

  // Extract debug option
  {
    const auto& debug_opts = parser.get_options(compiler::CompilerOption::Type::Debug);
    const auto& max_opt = std::max_element(debug_opts.begin(), debug_opts.end());

    // Default optimization
    if (max_opt == debug_opts.end()) {
      tool->optimizations.debug_level = compiler::DebugOption::get_default().second.atmel_studio_description;
    } else {
      tool->optimizations.debug_level = (*max_opt)->generate(true);
    }
  }

  // List "other" debugging flags (a.k.a unsupported flags)
  tool->optimizations.other_debugging_flags = get_unsupported_options(parser,
                                                                      compiler::CompilerOption::Type::Debug,
                                                                      tool->get_supported_debug_options());

  tool->warnings.all_warnings = parser.has_option("-Wall");
  tool->warnings.extra_warnings = parser.has_option("-Wextra");
  tool->warnings.undefined = parser.has_option("-Wundef");
  tool->warnings.warnings_as_error = parser.has_option("-Werror");
  tool->warnings.check_syntax_only = parser.has_option("-fsyntax-only");
  tool->warnings.pedantic = parser.has_option("-pedantic");
  tool->warnings.pedantic_warnings_as_errors = parser.has_option("-pedantic-errors");
  tool->warnings.inhibit_all_warnings = parser.has_option("-w");

  tool->miscellaneous.other_flags += get_unsupported_options(parser,
                                                             compiler::CompilerOption::Type::Warning,
                                                             tool->get_supported_warning_options());
  tool->miscellaneous.other_flags += " ";


  tool->miscellaneous.do_not_delete_temporary_files = parser.has_option("-save-temps");
  tool->miscellaneous.verbose = parser.has_option("-v");
  tool->miscellaneous.support_ansi_programs = parser.has_option("-ansi");

  // List "other" miscellaneous flags (a.k.a unsupported flags)
  compiler::cmAvrGccCompiler::OptionsVec language_standards = parser.get_options(compiler::CompilerOption::Type::LanguageStandard);
  for (auto& opt : language_standards) {
    compiler::LanguageStandardOption* lstd = static_cast<compiler::LanguageStandardOption*>(opt.get());
    if (lstd->lang.to_string() == lang) {
      tool->miscellaneous.other_flags += lstd->get_token() + " ";
    }
  }

  tool->miscellaneous.other_flags += get_unsupported_options(parser,
                                                             compiler::CompilerOption::Type::Generic,
                                                             get_all_supported_options());
  tool->miscellaneous.other_flags += " ";

  linker.general.do_not_use_default_libraries = parser.has_option("-nostartfile");
  linker.general.do_not_use_default_libraries = parser.has_option("-nodefaultlibs");
  linker.general.no_startup_or_default_libs = parser.has_option("-nostdlib");
  linker.general.omit_all_symbol_information = parser.has_option("-Wl,s");
  linker.general.no_shared_libraries = parser.has_option("-Wl,-static");
  linker.general.generate_map_file = parser.has_option("-Wl,-Map");
  linker.general.use_vprintf_library = parser.has_option("-Wl,-u,vprintf");

  linker.optimizations.garbage_collect_unused_sections = parser.has_option("-Wl,--gc-sections");
  linker.optimizations.put_read_only_data_in_writable_data_section = parser.has_option("--rodata-writable");

  // List "other" miscellaneous flags (a.k.a unsupported flags)
  linker.miscellaneous.linker_flags = get_unsupported_options(parser,
                                                              compiler::CompilerOption::Type::Linker,
                                                              linker.get_supported_options());

  assembler.debugging.debug_level = parser.has_option("-Wa,-g") ? "Default (-Wa,-g)" : "";
}

void xml_append_inline(pugi::xml_node& parent, const std::string& node_name, const std::string& value = "")
{
  pugi::xml_node node = parent.append_child(node_name.c_str());
  if (0 != value.size()) {
    pugi::xml_node child = node.append_child(pugi::node_pcdata);
    child.set_value(value.c_str());
  }
}

void AS7AvrGCC8::generate_xml_per_language(pugi::xml_node& parent, const std::string& toolname, const AS7AvrGcc8_Base& target) const
{
  xml_append_inline(parent, toolname + ".compiler.general.SubroutinesFunctionPrologues", target.general.subroutine_function_prologue ? "True" : "False");
  xml_append_inline(parent, toolname + ".compiler.general.ChangeSPWithoutDisablingInterrupts", target.general.change_stack_pointer_without_disabling_interrupt ? "True" : "False");
  xml_append_inline(parent, toolname + ".compiler.general.ChangeDefaultCharTypeUnsigned", target.general.change_default_chartype_unsigned ? "True" : "False");
  xml_append_inline(parent, toolname + ".compiler.general.ChangeDefaultBitFieldUnsigned", target.general.change_default_bitfield_unsigned ? "True" : "False");

  xml_append_inline(parent, toolname + ".compiler.preprocessor.DoNotSearchSystemDirectories", target.preprocessor.do_not_search_system_directories ? "True" : "False");
  xml_append_inline(parent, toolname + ".compiler.preprocessor.PreprocessOnly", target.preprocessor.preprocess_only ? "True" : "False");

  // Symbols definition
  if (!target.symbols.def_symbols.empty()) {
    pugi::xml_node defsymbols_node = parent.append_child((toolname + ".compiler.symbols.DefSymbols").c_str());
    pugi::xml_node list_values_node = defsymbols_node.append_child("ListValues");
    for (const auto& symbol : target.symbols.def_symbols) {
      xml_append_inline(list_values_node, "Value", symbol);
    }
  }

  // Include directories definition
  if (!target.directories.include_paths.empty()) {
    pugi::xml_node include_path_node = parent.append_child((toolname + ".compiler.directories.IncludePaths").c_str());
    pugi::xml_node list_values_node = include_path_node.append_child("ListValues");
    for (const auto& include : target.directories.include_paths) {
      xml_append_inline(list_values_node, "Value", include);
    }
  }

  // Optimizations and debug flags
  xml_append_inline(parent, toolname + ".compiler.optimization.level", target.optimizations.level);
  if (!target.optimizations.other_flags.empty()) {
    std::string other_flags = cmutils::strings::trim(target.optimizations.other_flags, ' ', cmutils::strings::TransformLocation::Both);
    xml_append_inline(parent, toolname + ".compiler.optimization.OtherFlags", other_flags);
  }

  xml_append_inline(parent, toolname + ".compiler.optimization.PrepareFunctionsForGarbageCollection", target.optimizations.prepare_function_for_garbage_collection ? "True" : "False");
  xml_append_inline(parent, toolname + ".compiler.optimization.PrepareDataForGarbageCollection", target.optimizations.prepare_data_for_garbage_collection ? "True" : "False");
  xml_append_inline(parent, toolname + ".compiler.optimization.PackStructureMembers", target.optimizations.pack_structure_members ? "True" : "False");
  xml_append_inline(parent, toolname + ".compiler.optimization.AllocateBytesNeededForEnum", target.optimizations.allocate_bytes_needed_for_enum ? "True" : "False");
  xml_append_inline(parent, toolname + ".compiler.optimization.UseShortCalls", target.optimizations.use_short_calls ? "True" : "False");
  xml_append_inline(parent, toolname + ".compiler.optimization.DebugLevel", target.optimizations.debug_level);

  if (!target.optimizations.other_debugging_flags.empty()) {
    std::string other_flags = cmutils::strings::trim(target.optimizations.other_debugging_flags, ' ', cmutils::strings::TransformLocation::Both);
    xml_append_inline(parent, toolname + ".compiler.optimization.OtherDebuggingFlags", other_flags);
  }

  // Warnings
  xml_append_inline(parent, toolname + ".compiler.warnings.AllWarnings", target.warnings.all_warnings ? "True" : "False");

  // Note : only avrgcc supports -Wextra in AtmelStudio7 ...
  if (toolname == "avrgcc") {
    xml_append_inline(parent, toolname + ".compiler.warnings.ExtraWarnings", target.warnings.extra_warnings ? "True" : "False");
  }
  xml_append_inline(parent, toolname + ".compiler.warnings.Undefined", target.warnings.undefined ? "True" : "False");
  xml_append_inline(parent, toolname + ".compiler.warnings.WarningsAsErrors", target.warnings.warnings_as_error ? "True" : "False");
  xml_append_inline(parent, toolname + ".compiler.warnings.CheckSyntaxOnly", target.warnings.check_syntax_only ? "True" : "False");
  xml_append_inline(parent, toolname + ".compiler.warnings.Pedantic", target.warnings.pedantic ? "True" : "False");
  xml_append_inline(parent, toolname + ".compiler.warnings.PedanticWarningsAsErrors", target.warnings.pedantic_warnings_as_errors ? "True" : "False");
  xml_append_inline(parent, toolname + ".compiler.warnings.InhibitAllWarnings", target.warnings.inhibit_all_warnings ? "True" : "False");

  // Miscellaneous flags
  if (!target.miscellaneous.other_flags.empty()) {
    std::string other_flags = cmutils::strings::trim(target.miscellaneous.other_flags, ' ', cmutils::strings::TransformLocation::Both);
    xml_append_inline(parent, toolname + ".compiler.miscellaneous.OtherFlags", other_flags);
  }

  xml_append_inline(parent, toolname + ".compiler.miscellaneous.Verbose", target.miscellaneous.verbose ? "True" : "False");
  xml_append_inline(parent, toolname + ".compiler.miscellaneous.SupportAnsiPrograms", target.miscellaneous.support_ansi_programs ? "True" : "False");
  xml_append_inline(parent, toolname + ".compiler.miscellaneous.DoNotDeleteTemporaryFiles", target.miscellaneous.do_not_delete_temporary_files ? "True" : "False");
}

void AS7AvrGCC8::generate_xml(pugi::xml_node& parent, const std::string& lang) const
{
  xml_append_inline(parent, "avrgcc.common.Device", common.Device);
  xml_append_inline(parent, "avrgcc.common.optimization.RelaxBranches", common.relax_branches ? "True" : "False");
  xml_append_inline(parent, "avrgcc.common.ExternalRamMemOvflw.", common.external_ram_mem_ovflw ? "True" : "False");
  xml_append_inline(parent, "avrgcc.common.outputfiles.hex", common.outputfiles.hex ? "True" : "False");
  xml_append_inline(parent, "avrgcc.common.outputfiles.lss", common.outputfiles.lss ? "True" : "False");
  xml_append_inline(parent, "avrgcc.common.outputfiles.eep", common.outputfiles.eep ? "True" : "False");
  xml_append_inline(parent, "avrgcc.common.outputfiles.srec", common.outputfiles.srec ? "True" : "False");
  xml_append_inline(parent, "avrgcc.common.outputfiles.usersignatures", common.outputfiles.usersignatures ? "True" : "False");

  generate_xml_per_language(parent, "avrgcc", avrgcc);
  std::string linker_ref = "avrgcc";
  if (lang != "C") {
    generate_xml_per_language(parent, "avrgcccpp", avrgcccpp);
    // Weird thing from AtmelStudio side, linker parameters use the last compiler name ...
    linker_ref = "avrgcccpp";
  }

  ////////////////////////////// Linker //////////////////////////////

  // Linker general informations
  xml_append_inline(parent, linker_ref + ".linker.general.DoNotUseStandardStartFiles", linker.general.do_not_use_standard_start_file ? "True" : "False");
  xml_append_inline(parent, linker_ref + ".linker.general.DoNotUseDefaultLibraries", linker.general.do_not_use_default_libraries ? "True" : "False");
  xml_append_inline(parent, linker_ref + ".linker.general.NoStartupOrDefaultLibs", linker.general.no_startup_or_default_libs ? "True" : "False");
  xml_append_inline(parent, linker_ref + ".linker.general.OmitAllSymbolInformation", linker.general.omit_all_symbol_information ? "True" : "False");
  xml_append_inline(parent, linker_ref + ".linker.general.NoSharedLibraries", linker.general.no_shared_libraries ? "True" : "False");

  // This one is some kind of a bug from Atmel Studio : only avrgcc can get this option mapped to!
  xml_append_inline(parent, "avrgcc.linker.general.GenerateMAPFile", linker.general.generate_map_file ? "True" : "False");
  xml_append_inline(parent, linker_ref + ".linker.general.UseVprintfLibrary", linker.general.use_vprintf_library ? "True" : "False");

  // Link libraries
  if (!linker.libraries.libraries.empty()) {
    pugi::xml_node link_libraries_node = parent.append_child((linker_ref + ".linker.libraries.Libraries").c_str());
    pugi::xml_node list_values_node = link_libraries_node.append_child("ListValues");
    for (const auto& lib : linker.libraries.libraries) {
      xml_append_inline(list_values_node, "Value", lib);
    }
  }

  // Link libraries search path
  if (!linker.libraries.search_path.empty()) {
    pugi::xml_node link_libraries_search_path_node = parent.append_child((linker_ref + ".linker.libraries.LibrarySearchPaths").c_str());
    pugi::xml_node list_values_node = link_libraries_search_path_node.append_child("ListValues");
    for (const auto& lib : linker.libraries.search_path) {
      xml_append_inline(list_values_node, "Value", lib);
    }
  }

  xml_append_inline(parent, linker_ref + ".linker.optimization.GarbageCollectUnusedSections", linker.optimizations.garbage_collect_unused_sections ? "True" : "False");
  xml_append_inline(parent, linker_ref + ".linker.optimization.PutReadOnlyDataInWritableDataSection", linker.optimizations.put_read_only_data_in_writable_data_section ? "True" : "False");

  if (!linker.miscellaneous.linker_flags.empty()) {
    std::string other_flags = cmutils::strings::trim(linker.miscellaneous.linker_flags, ' ', cmutils::strings::TransformLocation::Both);
    xml_append_inline(parent, linker_ref + ".linker.miscellaneous.LinkerFlags", other_flags);
  }

  ////////////////////////////// Assembler //////////////////////////////
  // Assembler include path
  if (!assembler.general.include_path.empty()) {
    pugi::xml_node assembler_include_path_node = parent.append_child((linker_ref + ".assembler.general.IncludePaths").c_str());
    pugi::xml_node list_values_node = assembler_include_path_node.append_child("ListValues");
    for (const auto& include : assembler.general.include_path) {
      xml_append_inline(list_values_node, "Value", include);
    }
  }
  xml_append_inline(parent, linker_ref + ".assembler.general.AnounceVersion", assembler.general.anounce_version ? " True" : "False");

  if (!assembler.debugging.debug_level.empty()) {
    xml_append_inline(parent, linker_ref + ".assembler.debugging.DebugLevel", assembler.debugging.debug_level);
  }

  // Archiver parameters
  if (archiver_flags != "-r") {
    xml_append_inline(parent, linker_ref + ".archiver.general.ArchiverFlags", archiver_flags);
  }
}

void Common::clear()
{
  Device.clear();
  relax_branches = false;
  external_ram_mem_ovflw = false;
  outputfiles.hex = false;
  outputfiles.lss = false;
  outputfiles.eep = false;
  outputfiles.srec = false;
  outputfiles.usersignatures = false;
}

std::vector<std::string> Common::get_supported_options() const
{
  const std::vector<std::string> out = {
    "-mrelax"
  };
  return out;
}

std::vector<std::string> AS7AvrGcc8_Base::get_supported_options() const
{
  std::vector<std::string> out;
  std::vector<std::string> general_opts = get_supported_general_options();
  std::vector<std::string> optimizations_opts = get_supported_optimizations_options();
  std::vector<std::string> misc_opts = get_supported_misc_options();
  std::vector<std::string> warning_opts = get_supported_warning_options();
  std::vector<std::string> preprocessor_opts = get_supported_preprocessor_options();

  out.insert(out.end(), general_opts.begin(), general_opts.end());
  out.insert(out.end(), optimizations_opts.begin(), optimizations_opts.end());
  out.insert(out.end(), misc_opts.begin(), misc_opts.end());
  out.insert(out.end(), warning_opts.begin(), warning_opts.end());
  out.insert(out.end(), general_opts.begin(), general_opts.end());
  out.insert(out.end(), preprocessor_opts.begin(), preprocessor_opts.end());

  return out;
}

std::vector<std::string> AS7AvrGcc8_Base::get_supported_general_options() const
{
  const std::vector<std::string> out = {
    "-mcall-prologues",
    "-mno-interrupts",
    "-funsigned-char",
    "-funsigned-bitfields",
  };
  return out;
}

std::vector<std::string> AS7AvrGcc8_Base::get_supported_optimizations_options() const
{
  const std::vector<std::string> out = {
    "-O0",
    "-O",
    "-O1",
    "-O2",
    "-O3",
    "-Os",
    "-Ofast",
    "-Og",
    "-ffunction-sections",
    "-fdata-sections",
    "-fpack-struct",
    "-fshort-enums",
    "-mshort-calls",
  };
  return out;
}

std::vector<std::string> AS7AvrGcc8_Base::get_supported_debug_options() const
{
  const std::vector<std::string> out = {
    "-g0",
    "-g",
    "-g1",
    "-g2",
    "-g3",
    "-ggdb",
    "-gdwarf"
  };
  return out;
}

std::vector<std::string> AS7AvrGcc8_Base::get_supported_warning_options() const
{
  const std::vector<std::string> out = {
    "-Wall",
    "-Wextra",
    "-Wundef",
    "-Werror",
    "-fsyntax-only",
    "-pedantic",
    "-pedantic-errors",
    "-w",
  };
  return out;
}

std::vector<std::string> AS7AvrGcc8_Base::get_supported_misc_options() const
{
  const std::vector<std::string> out = {
    "-v",
    "-ansi",
    "-save-temps"
  };
  return out;
}

std::vector<std::string> AS7AvrGcc8_Base::get_supported_preprocessor_options() const
{
  const std::vector<std::string> out = {
    "-nostdinc",
    "-E"
  };
  return out;
}

std::vector<std::string> AS7AvrGCC8Linker::get_supported_options() const
{
  std::vector<std::string> out;
  const std::vector<std::string> opt_vec = get_supported_optimizations_options();
  const std::vector<std::string> gen_vec = get_supported_general_options();
  out.insert(out.end(), opt_vec.begin(), opt_vec.end());
  out.insert(out.end(), gen_vec.begin(), gen_vec.end());
  return out;
}

std::vector<std::string> AS7AvrGCC8Linker::get_supported_optimizations_options() const
{
  const std::vector<std::string> out = {
    "-Wl,--gc-sections",
    "-E"
  };
  return out;
}

std::vector<std::string> AS7AvrGCC8Linker::get_supported_general_options() const
{
  const std::vector<std::string> out = {
    "-nostartfile",
    "-nodefaultlibs",
    "-nostdlib",
    "-Wl,s",
    "-Wl,-static",
    "-Wl,-Map",
    "-Wl,-u,vprintf", // FIXME : We will have issues with this one as this requires a special parsing, simple "split" function won't do!
  };
  return out;
}

std::vector<std::string> AS7AvrGCC8Assembler::get_supported_options() const
{
  const std::vector<std::string> out = {
    "-Wa,g",
  };
  return out;
}

}
