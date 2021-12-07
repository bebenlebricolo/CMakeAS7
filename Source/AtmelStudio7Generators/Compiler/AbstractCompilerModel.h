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

#ifndef ABSTRACT_COMPILER_MODEL_HEADER
#define ABSTRACT_COMPILER_MODEL_HEADER

#include <string>
#include <vector>
#include <memory>
#include "cmAvrGccCompilerOption.h"

namespace compiler
{

/**
 * @brief This class depicts a basic compiler model abstraction layer used by the Atmel Studio toolchains.
 * @details It serves as a bridge between CMake's internal data model and the toolchain's data model when
 * it comes to compiler cli frontend. This model is then used to parse and capture input flags as seen by the real
 * compiler through CMake.
 * However we need this layer in order to properly categorise the various supported flags, as some of them
 * get specific treatment from AtmelStudio properties pages.
 * Some of the flags and options are exposed through checkboxes in AtmelStudio, others are labeled as "other flags" and
 * treated as miscellaneous.
 */
class AbstractCompilerModel
{
public:
    inline virtual ~AbstractCompilerModel() noexcept {};

    /**
     * @brief uses input vector of raw tokens (i.e. parsed from command line or CMAKE flags) and
     * builds the appropriate option container to store each token representation.
     *
     * @param tokens : vector of raw tokens parsed from command line input
     */
    virtual void parse_flags(const std::vector<std::string>& tokens) = 0;

    /**
     * @brief parses options and flags from given string where options and flags are separated by
     * a whitespace (command shell argument separator) and builds compiler option representation for each
     * one of them.
     *
     * Each new compiler option - if valid - is stored within the adequate container and can be accessed
     * using the method get_option(token) or get_options(CompilerOption::Type::XXX)
     * @see get_option()
     * @see get_options()
     *
     * @param flags : concatenated string including all raw tokens to be parsed
     */
    virtual void parse_flags(const std::string& flags) = 0;

    using ShrdOption = std::shared_ptr<CompilerOption>;
    using OptionsVec = std::vector<ShrdOption>;

    /**
     * @brief Returns the adequate vector of options (each option being wrapped in a shared_ptr)
     * using the type parameter as a key
     *
     * Note : returned vector is a vector of shared_ptr<CompilerOption*>, so you can either
     * use the generic CompilerOption interface or static_cast<> it knowing the real type
     * of the object.
     * For instance : get_options(CompilerOption::Type::Debug) will return a vector of
     * shared_ptr<CompilerOption*> where all wrapped object is in fact a DebugOption*
     * so static_cast<DebugOption*>(CompilerOption*object) will work
     *
     * @param type  : indicates which flavor of CompilerOption is requested
     * @return a vector of shared_ptr wrapping a CompilerOption* pointer.
     */
    virtual OptionsVec get_options(const CompilerOption::Type type) const = 0;

    /**
     * @brief fetches a particular option using the raw token as a key.
     * Each CompilerOption object has a type field which can be used to retrieve the derived
     * class
     *
     * @param token : raw token used as a key
     * @return nullptr in case of failure (raw token has no stored CompilerOption representation) or
     *                 the found pointer in case this raw token has already been stored previously
     */
    virtual CompilerOption * get_option(const std::string& token) const = 0;

    /**
     * @brief Get a list of available options stored in memory using the CompilerOption::Type
     * as a key. Returned list is a vector of raw tokens as parsed from command line input.
     *
     * @param type  : the kind of option you want to know about
     * @return  a list of supported options for that kind of option
     */
    virtual std::vector<std::string> get_all_options(const CompilerOption::Type type) const = 0;

    /**
     * @brief returns a list of raw tokens contained within a collection of Options, minus the options that Atmel Studio supports.
     * This is used to compute the "Other flags" section to be filled within AtmelStudio7 project files
     *
     * @param   type        :   type of targeted options
     * @param   as7_options :   atmel studio 7 supported options list
     * @return  a vector of raw tokens
     */
    virtual std::vector<std::string> get_unsupported_options(const CompilerOption::Type type, const std::vector<std::string>& as7_options) const = 0;

    /**
     * @brief Tells whether this cmAvrGccCompiler instance has the requested option in memory
     * or not. It will search in all its options containers and perform a search on each one of them.
     *
     * @param option : raw token
     * @return true  : this instance has the requested option representation stored in memory
     *         false : no available representation of this option was found in memory
     */
    virtual bool has_option(const std::string& option) const = 0 ;

    /**
     * @brief Clears all memory storages.
     */
    virtual void clear() = 0;
};

}

#endif /* ABSTRACT_COMPILER_HEADER */
