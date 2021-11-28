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
#include <memory>
#include <unordered_map>
#include "cmAvrGccCompilerOption.h"

namespace compiler
{

/**
 * @brief the cmAvrGccCompiler class is a simple compiler abstraction (often refered to
 * as "compiler abstraction" in consuming code) whose role is mainly to parse command line input
 * options parsed from the CMakeLists.txt files and categorize them under the right store.
 * It could also provide some means to validate flags (this has yet to be implemented) as
 * options and flags need to comply with a handfull of naming rules to be accepted.
 */
class cmAvrGccCompiler
{
public:
    /**
     * @brief uses input vector of raw tokens (i.e. parsed from command line or CMAKE flags) and
     * builds the appropriate option container to store each token representation.
     *
     * @param tokens : vector of raw tokens parsed from command line input
     */
    void parse_flags(const std::vector<std::string>& tokens);

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
    void parse_flags(const std::string& flags);

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
    OptionsVec get_options(const CompilerOption::Type type) const;

    /**
     * @brief fetches a particular option using the raw token as a key.
     * Each CompilerOption object has a type field which can be used to retrieve the derived
     * class
     *
     * @param token : raw token used as a key
     * @return nullptr in case of failure (raw token has no stored CompilerOption representation) or
     *                 the found pointer in case this raw token has already been stored previously
     */
    CompilerOption * get_option(const std::string& token) const;

    /**
     * @brief Get a list of available options stored in memory using the CompilerOption::Type
     * as a key. Returned list is a vector of raw tokens as parsed from command line input.
     *
     * @param type  : the kind of option you want to know about
     * @return  a list of supported options for that kind of option
     */
    std::vector<std::string> get_all_options(const CompilerOption::Type type) const;

    /**
     * @brief returns a list of raw tokens contained within a collection of Options, minus the options that Atmel Studio supports.
     * This is used to compute the "Other flags" section to be filled within AtmelStudio7 project files
     *
     * @param   type        :   type of targeted options
     * @param   as7_options :   atmel studio 7 supported options list
     * @return  a vector of raw tokens
     */
    std::vector<std::string> get_unsupported_options(const CompilerOption::Type type, const std::vector<std::string>& as7_options) const;

    /**
     * @brief Tells whether this cmAvrGccCompiler instance has the requested option in memory
     * or not. It will search in all its options containers and perform a search on each one of them.
     *
     * @param option : raw token
     * @return true  : this instance has the requested option representation stored in memory
     *         false : no available representation of this option was found in memory
     */
    bool has_option(const std::string& option) const;

    /**
     * @brief Clears all memory storages.
     */
    void clear();

private:

    /**
     * @brief Packs all tools to manipulate stored options.
     */
    struct Options
    {
        Options();

        /**
         * @brief Tells whether a token is found within the options storage.
         * @param token : raw token used as a key
         * @return true  : token was found in storage
         *         false : token was not found
         */
        bool contains(const std::string& token) const;

        /**
         * @brief Tells if given token is found in selected reference storage options vector.
         *
         * @param token     : raw token used as a key
         * @param reference : selected vector of options used as a reference
         * @return true  : token was found in storage
         *         false : token was not found
         */
        bool contains(const std::string& token, const OptionsVec& reference) const;

        /**
         * @brief Fetches and retrieve a single option from storage
         *
         * @param token : raw token used as a key
         * @return non-empty shared_ptr<CompilerOption*> : a matching item was found
         *         empty shared_ptr<CompilerOption*> (==nullptr) : no match
         */
        ShrdOption get_option(const std::string& token) const;

        /**
         * @brief Fetches and retrieve a single option within selected storage vector.
         *
         * @param token : raw token used as a key
         * @param vect  : selected vector of options used as a reference
         * @return non-empty shared_ptr<CompilerOption*> : a matching item was found
         *         empty shared_ptr<CompilerOption*> (==nullptr) : no match
         */
        ShrdOption get_option(const std::string& token, const OptionsVec& vect) const;

        /**
         * @brief Tells if token is only found once in selected storage vector.
         *
         * @param token     : raw token used as a key
         * @param reference : selected vector of options used as a reference
         * @return true  : token is found only once in selected reference vector
         *         false : token is either not found or found multiple times in selected reference vector
         */
        bool is_unique(const std::string& token, const OptionsVec& reference) const;

        /**
         * @brief Tells if option is only found once in selected storage vector.
         *
         * @param option    : reference option to be seach for
         * @param reference : selected vector of options used as a reference
         * @return true  : option is found only once in selected reference vector
         *         false : option is either not found or found multiple times in selected reference vector
         */
        bool is_unique(const ShrdOption& option, const OptionsVec& reference) const;

        /**
         * @brief store given option in the right storage section depending on its type.
         *
         * @param option : option to be stored appropriately
         */
        void accept_option(const ShrdOption& option);

        /**
         * @brief Appends an option at the end of the selected vector, performing several checks
         * before adding to container (for instance, option has to be unique in the targeted vector,
         * otherwise it will be discarded).
         *
         * @param option : option to be stored
         * @param vec    : selected option vector
         */
        void push_option(const ShrdOption& option, OptionsVec& vec);

        /**
         * @brief Clears all memory.
         */
        void clear();

        std::unordered_map<CompilerOption::Type, OptionsVec> storage; /**< Stores multiple vectors of options of different kinds within a single map*/
    } options;
};

}
