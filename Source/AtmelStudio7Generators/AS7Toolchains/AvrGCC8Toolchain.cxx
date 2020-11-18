#include "AvrGCC8Toolchain.h"
#include "cmAvrGccCompiler.h"
#include "cmAvrGccOptimizationOption.h"
#include "cmAvrGccDebugOption.h"

#include "pugixml.hpp"

#include <algorithm>
namespace AvrToolchain
{

void AS7AvrGCC8::convert_from(const compiler::cmAvrGccCompiler& parser, const std::string& lang)
{
    //parser.get_options(compiler::CompilerOption::Type::Optimization)
    AS7AvrGcc8_Base * tool = nullptr;
    if (lang == "C")
    {
        tool = &avrgcc;
    }
    else
    {
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
        for (const auto& def : definitions)
        {
            tool->symbols.def_symbols.push_back(def->get_token());
        }
    }

    // Extract optimizations
    {
        const auto& optimizations = parser.get_options(compiler::CompilerOption::Type::Optimization);
        const auto& max_opt = std::max_element(optimizations.begin(), optimizations.end());

        // Default optimization
        if (max_opt == optimizations.end())
        {
            tool->optimizations.level = compiler::OptimizationOption::get_default().second.atmel_studio_description;
        }
        else
        {
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
        if (max_opt == debug_opts.end())
        {
            tool->optimizations.debug_level = compiler::DebugOption::get_default().second.atmel_studio_description;
        }
        else
        {
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

void AS7AvrGCC8::generate_xml(pugixml::xml_node& parent)
{

}


}