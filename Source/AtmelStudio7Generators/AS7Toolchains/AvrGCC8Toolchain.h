#pragma once

#include <string>
#include <vector>

#include "cmAvrGccCompilerOption.h"

namespace compiler {
class cmAvrGccCompiler;
}

namespace pugi {
class xml_node;
}

namespace AvrToolchain {

struct BasicRepresentation
{
public:
  virtual void clear() = 0;
  virtual std::vector<std::string> get_supported_options() const = 0;
};

struct Common : public BasicRepresentation
{
  std::string Device;
  bool relax_branches = false; /**< -mrelax    */
  bool external_ram_mem_ovflw = false;
  struct
  {
    bool hex = true;
    bool lss = true;
    bool eep = true;
    bool srec = true;
    bool usersignatures = false;
  } outputfiles;

  void clear() override;
  std::vector<std::string> get_supported_options() const override;
};

struct AS7AvrGcc8_Base : public BasicRepresentation
{
  AS7AvrGcc8_Base() = default;
  AS7AvrGcc8_Base(const AS7AvrGcc8_Base& other);

  void copy_from(const AS7AvrGcc8_Base& other);

  struct
  {
    bool subroutine_function_prologue = false;                     /**< -mcall-prologues       */
    bool change_stack_pointer_without_disabling_interrupt = false; /**< -mno-interrupts        */
    bool change_default_chartype_unsigned = true;                  /**< -funsigned-char        */
    bool change_default_bitfield_unsigned = true;                  /**< -funsigned-bitfields   */
  } general;

  struct
  {
    bool do_not_search_system_directories = false; /**< -nostdinc  */
    bool preprocess_only = false;                  /**< -E         */
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
    bool prepare_function_for_garbage_collection = true; /**< -ffunction-sections    */
    bool prepare_data_for_garbage_collection = true;     /**< -fdata-sections        */
    bool pack_structure_members = true;                  /**< -fpack-struct          */
    bool allocate_bytes_needed_for_enum = true;          /**< -fshort-enums          */
    bool use_short_calls = false;                        /**< -mshort-calls          */
    std::string debug_level;
    std::string other_debugging_flags;
  } optimizations;

  struct
  {
    bool all_warnings = true;                 /**< -Wall              */
    bool extra_warnings = true;               /**< -Wextra            */
    bool undefined = false;                   /**< -Wundef            */
    bool warnings_as_error = false;           /**< -Werror            */
    bool check_syntax_only = false;           /**< -fsyntax-only      */
    bool pedantic = false;                    /**< -pedantic          */
    bool pedantic_warnings_as_errors = false; /**< -pedantic-errors   */
    bool inhibit_all_warnings = false;        /**< -w                 */
  } warnings;

  struct
  {
    std::string other_flags;
    bool verbose = false;                       /**< -v          */
    bool support_ansi_programs = false;         /**< -ansi       */
    bool do_not_delete_temporary_files = false; /**< -save-temps */
  } miscellaneous;

  void clear() override;
  std::vector<std::string> get_supported_options() const override;
  std::vector<std::string> get_supported_misc_options() const;
  std::vector<std::string> get_supported_warning_options() const;
  std::vector<std::string> get_supported_optimizations_options() const;
  std::vector<std::string> get_supported_debug_options() const;
  std::vector<std::string> get_supported_preprocessor_options() const;
  std::vector<std::string> get_supported_general_options() const;
};

struct AS7AvrGCC8Linker : public BasicRepresentation
{
  struct
  {
    bool do_not_use_standard_start_file = false; /**< -nostartfile   */
    bool do_not_use_default_libraries = false;   /**< -nodefaultlibs */
    bool no_startup_or_default_libs = false;     /**< -nostdlib      */
    bool omit_all_symbol_information = false;    /**< -Wl,s          */
    bool no_shared_libraries = true;             /**< -Wl,-static    */
    bool generate_map_file = true;               /**< -Wl,-Map       */
    bool use_vprintf_library = false;            /**< -Wl,-u,vprintf */
  } general;

  struct
  {
    std::vector<std::string> libraries;
    std::vector<std::string> search_path;
  } libraries;

  struct
  {
    bool garbage_collect_unused_sections = true;              /**< -Wl,--gc-sections  */
    bool put_read_only_data_in_writable_data_section = false; /**< --rodata-writable  */
  } optimizations;

  struct
  {
    std::string linker_flags;
  } miscellaneous;

  void clear() override;
  std::vector<std::string> get_supported_options() const override;
  std::vector<std::string> get_supported_general_options() const;
  std::vector<std::string> get_supported_optimizations_options() const;
};

struct AS7AvrGCC8Assembler : public BasicRepresentation
{
  struct
  {
    std::vector<std::string> include_path;
    bool anounce_version = false;
  } general;

  struct
  {
    std::string debug_level;
  } debugging;

  void clear() override;
  std::vector<std::string> get_supported_options() const override;
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
  void generate_xml(pugi::xml_node& parent, const std::string& lang = "C") const;
  void generate_xml_per_language(pugi::xml_node& parent, const std::string& toolname, const AS7AvrGcc8_Base& target) const;
  void clear();

private:
  std::string get_unsupported_options(const compiler::cmAvrGccCompiler& parser,
                                      const compiler::CompilerOption::Type type,
                                      const std::vector<std::string>& options) const;
};

}