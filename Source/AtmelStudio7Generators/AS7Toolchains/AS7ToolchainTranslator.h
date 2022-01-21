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
#include <unordered_map>

#include "AvrGCC8Toolchain.h"
#include "AbstractCompilerModel.h"

namespace pugi
{
    class xml_node;
}

namespace AvrToolchain
{

/**
 * @brief the AS7ToolchainTranslator class provides a simple way to parse input command line options for several
 * languages (C, C++ adn ASM) and to generate the AS7 xml representation for them.
 */
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
    compiler::AbstractCompilerModel* get_compiler(const std::string& lang);

    /**
     * @brief returns targeted language. This method resolves the "highest" language used in
     * terms of abstractions and returns it.
     * E.g. : C++ > C > ASM
     *
     * @return the "highest" level language found in the parsed ones.
     */
    std::string get_targeted_language() const;

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
    std::unordered_map <std::string, std::unique_ptr<compiler::AbstractCompilerModel>> compilers; /**< Collection of compiler abstractions        */
};


}