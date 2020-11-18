#pragma once

#include <string>
#include <vector>

namespace compiler
{
    struct cmAvrGccCompiler;
}

namespace AvrToolchain
{
namespace pugixml
{
    class xml_node;
}

struct Common
{
    std::string Device;
    bool relax_branches = false;        /**< -mrelax    */
    bool ExternalRamOvflw = false;
    struct
    {
        bool hex = true;
        bool lss = true;
        bool eep = true;
        bool srec = true;
        bool usersignature = false;
    } outputfiles;
};

struct AS7AvrGcc8_Base
{
    struct
    {
        bool subroutine_function_prologue = false;                      /**< -mcall-prologues       */
        bool change_stack_pointer_without_disabling_interrupt = false;  /**< -mno-interrupts        */
        bool change_default_chartype_unsigned = true;                   /**< -funsigned-char        */
        bool change_default_bitfield_unsigned = true;                   /**< -funsigned-bitfields   */
    } general;

    struct
    {
        bool do_not_search_system_directories = false;  /**< -nostdinc  */
        bool preprocess_only = false;                   /**< -E         */
    } preprocessor;

    // Used to generate preprocessor's definitions
    struct
    {
        std::vector<std::string> def_symbols;
    } symbols;

    // Used to generate Include directories
    struct
    {
        std::vector<std::string> include_paths;
    } directories;

    struct
    {
        std::string level;
        std::string other_flags;
        bool prepare_function_for_garbage_collection = true;    /**< -ffunction-sections    */
        bool prepare_data_for_garbage_collection = true;        /**< -fdata-sections        */
        bool pack_structure_members = true;                     /**< -fpack-struct          */
        bool allocate_bytes_needed_for_enum = true;             /**< -fshort-enums          */
        bool use_short_calls = false;                           /**< -mshort-calls          */
        std::string debug_level;
        std::string other_debugging_flags;
    } optimizations;

    struct
    {
        bool all_warnings = true;                   /**< -Wall              */
        bool extra_warnings = true;                 /**< -Wextra            */
        bool undefined = false;                     /**< -Wundef            */
        bool warnings_as_error = false;             /**< -Werror            */
        bool check_syntax_only = false;             /**< -fsyntax-only      */
        bool pedantic = false;                      /**< -pedantic          */
        bool pedantic_warnings_as_errors = false;   /**< -pedantic-errors   */
        bool inhibit_all_warnings = false;          /**< -w                 */
    } warnings;

    struct
    {
        std::string other_flags;
        bool verbose = false;                           /**< -v          */
        bool support_ansi_programs = false;             /**< -ansi       */
        bool do_not_delete_temporary_files = false;     /**< -save-temps */
    } miscellaneous;
};

struct AS7AvrGCC8Linker
{
    struct
    {
        bool do_not_use_standard_start_file = false;    /**< -nostartfile   */
        bool do_not_use_default_libraries = false;      /**< -nodefaultlibs */
        bool no_startup_or_default_libs = false;        /**< -nostdlib      */
        bool omit_all_symbol_information = false;       /**< -Wl,s          */
        bool no_shared_libraries = true;                /**< -Wl,-static    */
        bool generate_map_file = true;                  /**< -Wl,-Map       */
        bool use_vprintf_library = false;               /**< -Wl,-u,vprintf */
    } general;

    struct
    {
        std::vector<std::string> libraries;
    } libraries;

    struct
    {
        bool garbage_collect_unused_sections = true;                /**< -Wl,--gc-sections  */
        bool put_read_only_data_in_writable_data_section = false;   /**< --rodata-writable  */
    } optimizations;

    struct
    {
        std::string linker_flags;
    } miscellaneous;
};

struct AS7AvrGCC8Assembler
{
    struct
    {
        std::vector<std::string> include_path;
    } general;

    struct
    {
        std::string debug_level;
    } debugging;
};

struct AS7AvrGCC8
{
    Common common;
    AS7AvrGcc8_Base avrgcc;
    AS7AvrGcc8_Base avrgcccpp;
    AS7AvrGCC8Linker linker;
    AS7AvrGCC8Assembler assembler;
    std::string archiver_flags = "-r";

    void convert_from(const compiler::cmAvrGccCompiler& parser, const std::string& lang = "C");
    void generate_xml(pugixml::xml_node& parent);
    pugixml::xml_node generate_xml();
};

}