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

#pragma once

#include <string>
#include <vector>
#include "cmAvrGccCompilerOption.h"

namespace compiler {
  class AbstractCompilerModel;
}

namespace pugi {
  class xml_node;
}

namespace AvrToolchain {

/**
 * @brief This sructure provides a basic representation of supported options of AS7.
 */
struct BasicRepresentation
{
public:
  /**
   * @brief Clears memory (resets options to default).
   */
  virtual void clear() = 0;

  /**
   * @brief Returns a vector of the options supported by AS7 for this class.
   * @return vector of strings
   */
  virtual std::vector<std::string> get_supported_options() const = 0;
  virtual ~BasicRepresentation() = default;
};

/**
 * @brief Common structure encodes the general options supported by AS7 which
 * are common to all languages.
 */
struct Common : public BasicRepresentation
{
  ~Common()
  {
    clear();
  }
  std::string Device;                   /**< Device name using AS7 naming conventions               */
  bool relax_branches = false;          /**< -mrelax                                                */
  bool external_ram_mem_ovflw = false;  /**< No compiler options is available for this one
                                             AS7 will perform a memory check to verify there is no
                                             external RAM overflow                                  */
  /**
   * @brief depicts the output files that Atmel Studio 7 can generate.
   */
  struct
  {
    bool hex = true;    /**< A .hex file will be produced by this project                       */
    bool lss = true;    /**< A .lss file (mix of source code and assembly language)             */
    bool eep = true;    /**< A .eep (eeprom dump) file will be produced by this project         */
    bool srec = true;   /**< A .srec file will be produced by this project                      */
    bool usersignatures = false; /**< might be used to encode serial numbers and other things   */
  } outputfiles;

  /**
   * @brief resets output files to their default state and clears the Device field.
   */
  void clear() override;

  /**
   * @brief Returns a list of supported options from command line input.
   * @return vector of string
   */
  std::vector<std::string> get_supported_options() const override;
};

/**
 * @brief AS7AvrGcc8_Base depicts basic options supported by C and C++ avr-gcc frontend for 8 bit
 * targets only.
 */
struct AS7AvrGcc8_Base : public BasicRepresentation
{
  AS7AvrGcc8_Base() = default;
  AS7AvrGcc8_Base(const std::string& lang_standard);
  AS7AvrGcc8_Base(const AS7AvrGcc8_Base& other);

  /**
   * @brief Deep copies the content of another structure.
   * @param other   : other structure
   */
  void copy_from(const AS7AvrGcc8_Base& other);

  /**
   * @brief depicts general options supported by AS7 C/C++ frontend.
   */
  struct
  {
    bool subroutine_function_prologue = false;                     /**< -mcall-prologues       */
    bool change_stack_pointer_without_disabling_interrupt = false; /**< -mno-interrupts        */
    bool change_default_chartype_unsigned = true;                  /**< -funsigned-char        */
    bool change_default_bitfield_unsigned = true;                  /**< -funsigned-bitfields   */
  } general;

  /**
   * @brief depicts preprocessor options supported by AS7 C/C++ frontend.
   */
  struct
  {
    bool do_not_search_system_directories = false; /**< -nostdinc  */
    bool preprocess_only = false;                  /**< -E         */
  } preprocessor;

  /**
   * @brief handles preprocessor's definitions
   */
  struct
  {
    std::vector<std::string> def_symbols; /**< list of symbols (-DSYMBOL=VALUE) records */
  } symbols;

  /**
   * @brief handles Include directories
   */
  struct
  {
    std::vector<std::string> include_paths; /**< list of include directories paths for avr-gcc compiler */
  } directories;

  /**
   * @brief Handles optimization related options.
   */
  struct
  {
    std::string level;                                   /**< Plain string encoding the optimization level (-Oxxx)   */
    std::string other_flags;                             /**< Other optimization flags not supported by AS7 frontend */
    bool prepare_function_for_garbage_collection = true; /**< -ffunction-sections    */
    bool prepare_data_for_garbage_collection = true;     /**< -fdata-sections        */
    bool pack_structure_members = true;                  /**< -fpack-struct          */
    bool allocate_bytes_needed_for_enum = true;          /**< -fshort-enums          */
    bool use_short_calls = false;                        /**< -mshort-calls          */
    std::string debug_level;                             /**< Plain string encoding the Debugging level (-gxxx)   */
    std::string other_debugging_flags;                   /**< Other debugging flags not supported by AS7 frontend */
  } optimizations;

  /**
   * @brief Handles Warning related options.
   */
  struct
  {
    bool all_warnings = true;                 /**< -Wall                                                            */
    bool extra_warnings = true;               /**< -Wextra                                                          */
    bool undefined = false;                   /**< -Wundef                                                          */
    bool warnings_as_error = false;           /**< -Werror                                                          */
    bool check_syntax_only = false;           /**< -fsyntax-only                                                    */
    bool pedantic = false;                    /**< -pedantic                                                        */
    bool pedantic_warnings_as_errors = false; /**< -pedantic-errors                                                 */
    bool inhibit_all_warnings = false;        /**< -w                                                               */
    std::string other_warnings;               /**< Any other warning flag passed to the compiler
                                                   This one does not appear in the generated xml as extra warnings
                                                   but instead they are packed under the miscellaneous flags        */
  } warnings;

  /**
   * @brief Handles other options which are not supported by any other fields.
   */
  struct
  {
    std::string other_flags;                    /**< any other options parsed by avr-gcc compiler representations but not supported
                                                     by Atmel Studio 7 IDE                                                          */
    bool verbose = false;                       /**< -v          */
    bool support_ansi_programs = false;         /**< -ansi       */
    bool do_not_delete_temporary_files = false; /**< -save-temps */
  } miscellaneous;

  /**
   * @brief Resets all options to their default state and clears all collections of
   * strings such as the unsupported flags list or plain strings such as optimization levels, etc.
   */
  void clear() override;

  /**
   * @brief gives the list of supported options.
   * All specific options list are concatenated to generate this one.
   * @return the list of all the options supported by this structure
   */
  std::vector<std::string> get_supported_options() const override;

  /**
   * @brief returns a list of all miscellaneous options
   */
  std::vector<std::string> get_supported_misc_options() const;

  /**
   * @brief returns a list of all warning options
   */
  std::vector<std::string> get_supported_warning_options() const;

  /**
   * @brief returns a list of all optimization options
   */
  std::vector<std::string> get_supported_optimizations_options() const;

  /**
   * @brief returns a list of all debug options
   */
  std::vector<std::string> get_supported_debug_options() const;

  /**
   * @brief returns a list of all preprocessor options
   */
  std::vector<std::string> get_supported_preprocessor_options() const;

  /**
   * @brief returns a list of all general options
   */
  std::vector<std::string> get_supported_general_options() const;
};

/**
 * @brief Depicts the supported options in AS7 frontend for avr-ld linker.
 */
struct AS7AvrGCC8Linker : public BasicRepresentation
{
  /**
   * @brief standard linker options.
   */
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

  /**
   * @brief handles linked libraries and linker search path.
   */
  struct
  {
    std::vector<std::string> libraries;   /**< list of libraries to link with (using the project's name)*/
    std::vector<std::string> search_path; /**< list of search path used by the linker at link time      */
  } libraries;

  /**
   * @brief handles linker optimizations.
   */
  struct
  {
    bool garbage_collect_unused_sections = true;              /**< -Wl,--gc-sections  */
    bool put_read_only_data_in_writable_data_section = false; /**< --rodata-writable  */
  } optimizations;

  /**
   * @brief handles other linker options which are not supported by AS7 frontend.
   */
  struct
  {
    std::string linker_flags; /**< List of linker options */
  } miscellaneous;

  /**
   * @brief clears internal memory and resets all options to their default values.
   */
  void clear() override;

  /**
   * @brief Returns all supported options for this AS7 linker frontend support.
   * The returned list is a concatenation of its general options and optimization options vectors
   */
  std::vector<std::string> get_supported_options() const override;

  /**
   * @brief Returns all supported AS7 linker general options
   */
  std::vector<std::string> get_supported_general_options() const;

  /**
   * @brief Returns all supported AS7 linker optimizations options
   */
  std::vector<std::string> get_supported_optimizations_options() const;
};

/**
 * @brief Depicts the supported options in AS7 frontend for avr assembler.
 */
struct AS7AvrGCC8Assembler : public BasicRepresentation
{
  /**
   * @brief handles general options for avr assembler.
   */
  struct
  {
    std::vector<std::string> include_path; /**< Lists assembler include path                 */
    bool anounce_version = false;          /**< outputs the Assembler version when compiling */
  } general;

  /**
   * @brief handles debugging options for avr assembler.
   */
  struct
  {
    std::string debug_level; /**< Gives the debugging level in a plain string form */
  } debugging;

  /**
   * @brief Clears memory and resets options to their default state.
   */
  void clear() override;

  /**
   * @brief Returns a list of all supported options for this assembler frontend.
   */
  std::vector<std::string> get_supported_options() const override;
};

/**
 * @brief aggregates all required components to form the adequate AS7 toolchain representation.
 */
struct AS7AvrGCC8
{
  Common common;                                /**< Common options                     */
  AS7AvrGcc8_Base avrgcc{ "-std=c89" };         /**< Options for avr-gcc (C mode)       */
  AS7AvrGcc8_Base avrgcccpp{ "-std=c++98" };    /**< Options for avr-g++ (C++ mode)     */
  AS7AvrGCC8Linker linker;                      /**< Options for avr-ld                 */
  AS7AvrGCC8Assembler assembler;                /**< Options for avr assembler          */
  std::string archiver_flags = "-r";            /**< Default archiver (avr-ar) flags    */

  /**
   * @brief Converts cmAvrGccCompiler abstraction (options parsed from input) to a valid AS7
   * toolchain representation.

   * All fields of AS7AvrGcc8 object are filled using cmAvrGccCompiler parsed options, which
   * are remapped knowing the list of AS7 supported options in each category (common, avrgcc, etc.)
   * Unresolved options (the one which does not fall into any existing field of AS7AvrGcc8 structure)
   * are automatically mapped into the "other flags" sections available in all categories.
   * This allows to preserve flags that could not be mapped internally, enabling Atmel Studio7
   * representation to match CMakeLists.txt project description as closely as possible independently
   * of the tools used.
   *
   * @param abstraction : compiler abstraction (collection of options parsed from command line input)
   * @param lang        : language for which the conversion is operated. Used to distinguish
   *                      avr gcc/g++.
   */
  void convert_from(const compiler::AbstractCompilerModel& abstraction, const std::string& lang = "C");

  /**
   * @brief Generates an AtmelStudio7 compatible XML representation of mapped options.
   *
   * Internal options are used to generate this XML file and every option will be serialized.
   * Natively, AtmelStudio7 does not serialize the options which are still in their default state.
   * However, this method does serialize all options, independently of their state being the default
   * one or not. This behavior is supported by AtmelStudio and does not break the parsing process.
   *
   * Depending of the input language, parts of AS7AvrGcc8 class will be used to generate the xml file
   * or the whole class might be used :
   *
   * [LANG] | [GENERATED LANGUAGES]
   *  CXX   | ASM, C and C++
   *  C     | ASM and C
   *  ASM   | only ASM
   *
   * @param parent  : parent xml node
   * @param lang    : the highest level language selected.
   */
  void generate_xml(pugi::xml_node& parent, const std::string& lang = "C") const;

  /**
   * @brief Clears memory and resets all options to their default state
   * + clears vectors and plain strings as well
   */
  void clear();

private:
  /**
   * @brief selectively generates a language representation using the adequate node name (aka toolname).
   *
   * @param parent      : parent xml node
   * @param toolname    : toolname used as the main "xml node". E.g : "C lang -> <AvrGcc> node, C++ lang -> <AvrGccCpp> node)
   * @param target      : targeted AvrGcc8_base structure to be serialized
   */
  void generate_xml_per_language(pugi::xml_node& parent, const std::string& toolname, const AS7AvrGcc8_Base& target) const;

  /**
   * @brief Returns a list of unsupported options parsed from command line input for a particular
   * option type (e.g. DebugOption or WarningOption).
   *
   * This allows to precisely resolve which option is not supported and requires special handling.
   * Those unsupported options are then used to fill the right section of the AS7AvrGcc8 class.
   * For instance, the option -Wl,--relax is not supported by AtmelStudio7 frontend, but is a
   * valid linker option. So this option will be added to the linker.miscellaneous.other_flags string
   * and passed to the toolchain at build time.
   *
   * This method basically retrieves a big list of all options supported by AS7 frontend and
   * compares each options provided by cmAvrGccCompiler representation.
   * Any time an option does not match the supported option list, it is added to the unsupported
   * options list.
   * Finally, this list is concatenated into a single string which could then be used to fill any of the
   * "other flags" sections of avrgcc, avrgcccpp, linker and assembler subclasses.
   *
   * @param abstraction : compiler abstraction used to retrieve parsed options
   * @param type        : Compiler option type. Used to determine the right "section" to fill the right tools
   * @param options     : list of known supported options
   * @example :
   * std::string out = get_unsupported_options(abstraction, compiler::CompilerOption::Type::Linker, {"-Wl,--gc-sections", "-Wl,--relax"};
   * will give : out == "-Wl,--relax"
   *
   * @return a plain string with a list of unsupported options separated by a whitespace ' '
   */
  std::string get_unsupported_options(const compiler::AbstractCompilerModel& abstraction,
                                      const compiler::CompilerOption::Type type,
                                      const std::vector<std::string>& options) const;

  /**
   * @brief Retrieves the list of absolutely all supported options, independently of the
   * tool being used (this is a concatenation of common, linker, assembler and avrgcc supported options).
   * Note : as avrgcc and avrgcccpp tools embeds exactly the same options, only one of them is used
   * to generate the options supported for both compiler frontends.
   * @return
   */
  std::vector<std::string> get_all_supported_options() const;
};

}
