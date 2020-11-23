#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "AvrGCC8Toolchain.h"
#include "cmAvrGccCompiler.h"

namespace pugi
{
    class xml_node;
}

namespace AvrToolchain
{

class AS7ToolchainTranslator
{
public:

    AS7ToolchainTranslator() = default;

    /**
     * @brief Wraps the compiler abstractions and toolchains to ease their use
     * @param   flags   :   vector of flags parsed from CMakeLists.txt files
     * @param   lang    :   Targeted language associated to given flag list
    */
    void parse(const std::vector<std::string>& flags, const std::string& lang = "C");

    /**
     * @brief Generates an xml representing the current toolchain using Atmel Studio 7 compatible format
     * @param   parent  :   parent node of Xml representation
    */
    void generate_xml(pugi::xml_node& parent);

    /**
     * @brief gives a matching compiler abstraction for a given language
     * @param   lang    :   targeted language
     * @return
     *      pointer to a compiler abstraction if one exists for selected language
     *      nullptr if no compiler abstraction could be found
    */
    compiler::cmAvrGccCompiler* get_compiler(const std::string& lang);

    /**
     * @brief Clears memory
    */
    void clear();

    AvrToolchain::AS7AvrGCC8 toolchain;    /**< Toolchain representation of parsed flags   */
private:

    /**
     * @brief Synchronizes C and C++ abstractions when C is missing and C++ is defined so that C abstraction
     * is filled using C++ one
    */
    void sync_toolchain_languages();

    /**
     * @brief translates compiler abstraction's content into Atmel Studio compatible format.
     * This function relies on toolchain.convert_from.
    */
    void translate();

    std::string targeted_language;   /**< Stores the language used. If C++ was given,
                                         then C and C++ abstractions will be used.
                                         Otherwise, only C is activated                 */
    std::unordered_map <std::string, compiler::cmAvrGccCompiler> compilers; /**< Collection of compiler abstractions        */
};


}