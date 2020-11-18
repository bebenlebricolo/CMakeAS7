#include "AvrGCC8Toolchain.h"

#include <algorithm>

#include "cmAvrGccCompiler.h"
#include "cmAvrGccDebugOption.h"
#include "cmAvrGccOptimizationOption.h"

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

void AS7AvrGCC8::convert_from(const compiler::cmAvrGccCompiler& parser, const std::string& lang)
{
  //parser.get_options(compiler::CompilerOption::Type::Optimization)
  AS7AvrGcc8_Base* tool = nullptr;
  if (lang == "C") {
    tool = &avrgcc;
  } else {
    tool = &avrgcccpp;
  }

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
      tool->symbols.def_symbols.push_back(def->get_token());
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

  tool->warnings.all_warnings = parser.has_option("-Wall");
  tool->warnings.extra_warnings = parser.has_option("-Wextra");
  tool->warnings.undefined = parser.has_option("-Wundef");
  tool->warnings.warnings_as_error = parser.has_option("-Werror");
  tool->warnings.check_syntax_only = parser.has_option("-fsyntax-only");
  tool->warnings.pedantic = parser.has_option("-pedantic");
  tool->warnings.pedantic_warnings_as_errors = parser.has_option("-pedantic-errors");
  tool->warnings.inhibit_all_warnings = parser.has_option("-w");

  tool->miscellaneous.do_not_delete_temporary_files = parser.has_option("-save-temps");
  tool->miscellaneous.verbose = parser.has_option("-v");
  tool->miscellaneous.support_ansi_programs = parser.has_option("-ansi");
  // TODO : resolve flags that weren't parsed already and add them to miscellaneous flags

  linker.general.do_not_use_default_libraries = parser.has_option("-nostartfile");
  linker.general.do_not_use_default_libraries = parser.has_option("-nodefaultlibs");
  linker.general.no_startup_or_default_libs = parser.has_option("-nostdlib");
  linker.general.omit_all_symbol_information = parser.has_option("-Wl,s");
  linker.general.no_shared_libraries = parser.has_option("-Wl,-static");
  linker.general.generate_map_file = parser.has_option("-Wl,-Map");
  linker.general.use_vprintf_library = parser.has_option("-Wl,-u,vprintf");

  linker.optimizations.garbage_collect_unused_sections = parser.has_option("-Wl,--gc-sections");
  linker.optimizations.put_read_only_data_in_writable_data_section = parser.has_option("--rodata-writable");

  // TODO : resolve other linker flags that weren't consumed already

  //assembler.debugging.debug_level
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

  xml_append_inline(parent, toolname + ".compiler.preprocessor.PreprocessOnly", target.preprocessor.preprocess_only ? "True" : "False");

  // Symbols definition
  {

    pugi::xml_node defsymbols_node = parent.append_child((toolname + ".compiler.symbols.DefSymbols").c_str());
    pugi::xml_node list_values_node = defsymbols_node.append_child("ListValues");
    for (const auto& symbol : target.symbols.def_symbols) {
      xml_append_inline(list_values_node, "Value", symbol);
    }
  }

  // Include directories definition
  {

    pugi::xml_node include_path_node = parent.append_child((toolname + ".compiler.directories.IncludePaths").c_str());
    pugi::xml_node list_values_node = include_path_node.append_child("ListValues");
    for (const auto& include : target.directories.include_paths) {
      xml_append_inline(list_values_node, "Value", include);
    }
  }

  // Optimizations and debug flags
  xml_append_inline(parent, toolname + ".compiler.optimization.level", target.optimizations.level);
  xml_append_inline(parent, toolname + ".compiler.optimization.OtherFlags", target.optimizations.other_flags);
  xml_append_inline(parent, toolname + ".compiler.optimization.PrepareFunctionsForGarbageCollection", target.optimizations.prepare_function_for_garbage_collection ? "True" : "False");
  xml_append_inline(parent, toolname + ".compiler.optimization.PrepareDataForGarbageCollection", target.optimizations.prepare_data_for_garbage_collection ? "True" : "False");
  xml_append_inline(parent, toolname + ".compiler.optimization.PackStructureMembers", target.optimizations.pack_structure_members ? "True" : "False");
  xml_append_inline(parent, toolname + ".compiler.optimization.AllocateBytesNeededForEnum", target.optimizations.allocate_bytes_needed_for_enum ? "True" : "False");
  xml_append_inline(parent, toolname + ".compiler.optimization.UseShortCalls", target.optimizations.use_short_calls ? "True" : "False");
  xml_append_inline(parent, toolname + ".compiler.optimization.DebugLevel", target.optimizations.debug_level);
  xml_append_inline(parent, toolname + ".compiler.optimization.OtherDebuggingFlags", target.optimizations.other_debugging_flags);

  // Warnings
  xml_append_inline(parent, toolname + ".compiler.warnings.AllWarnings", target.warnings.all_warnings ? "True" : "False");
  xml_append_inline(parent, toolname + ".compiler.warnings.ExtraWarnings", target.warnings.extra_warnings ? "True" : "False");
  xml_append_inline(parent, toolname + ".compiler.warnings.Undefined", target.warnings.undefined ? "True" : "False");
  xml_append_inline(parent, toolname + ".compiler.warnings.WarningsAsErrors", target.warnings.warnings_as_error ? "True" : "False");
  xml_append_inline(parent, toolname + ".compiler.warnings.CheckSyntaxOnly", target.warnings.check_syntax_only ? "True" : "False");
  xml_append_inline(parent, toolname + ".compiler.warnings.Pedantic", target.warnings.pedantic ? "True" : "False");
  xml_append_inline(parent, toolname + ".compiler.warnings.PedanticWarningsAsErrors", target.warnings.pedantic_warnings_as_errors ? "True" : "False");
  xml_append_inline(parent, toolname + ".compiler.warnings.InhibitAllWarnings", target.warnings.inhibit_all_warnings ? "True" : "False");

  // Miscellaneous flags
  xml_append_inline(parent, toolname + ".compiler.miscellaneous.OtherFlags", target.miscellaneous.other_flags);
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
  {
    pugi::xml_node link_libraries_node = parent.append_child((linker_ref + ".linker.libraries.Libraries").c_str());
    pugi::xml_node list_values_node = link_libraries_node.append_child("ListValues");
    for (const auto& lib : linker.libraries.libraries) {
      xml_append_inline(list_values_node, "Value", lib);
    }
  }

  // Link libraries search path
  {
    pugi::xml_node link_libraries_search_path_node = parent.append_child((linker_ref + ".linker.libraries.LibrarySearchPaths").c_str());
    pugi::xml_node list_values_node = link_libraries_search_path_node.append_child("ListValues");
    for (const auto& lib : linker.libraries.search_path) {
      xml_append_inline(list_values_node, "Value", lib);
    }
  }

  xml_append_inline(parent, linker_ref + ".linker.optimization.GarbageCollectUnusedSections", linker.optimizations.garbage_collect_unused_sections ? "True" : "False");
  xml_append_inline(parent, linker_ref + ".linker.optimization.PutReadOnlyDataInWritableDataSection", linker.optimizations.put_read_only_data_in_writable_data_section ? "True" : "False");
  xml_append_inline(parent, linker_ref + ".linker.miscellaneous.LinkerFlags", linker.miscellaneous.linker_flags);

  ////////////////////////////// Assembler //////////////////////////////
  // Assembler include path
  {
    pugi::xml_node assembler_include_path_node = parent.append_child((linker_ref + ".assembler.general.IncludePaths").c_str());
    pugi::xml_node list_values_node = assembler_include_path_node.append_child("ListValues");
    for (const auto& include : assembler.general.include_path) {
      xml_append_inline(list_values_node, "Value", include);
    }
  }
  xml_append_inline(parent, linker_ref + ".assembler.general.AnounceVersion", assembler.general.anounce_version ? " True" : "False");
  xml_append_inline(parent, linker_ref + ".assembler.debugging.DebugLevel", assembler.debugging.debug_level);

  // Archiver parameters
  xml_append_inline(parent, linker_ref + ".archiver.general.ArchiverFlags", archiver_flags);
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

}