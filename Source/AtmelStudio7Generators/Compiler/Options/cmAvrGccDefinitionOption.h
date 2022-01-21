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

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include "cmAvrGccCompilerOption.h"

namespace compiler {

/**
 * @brief Definition option provides services to handle -DCOMMAND_LINE_DEFINE=somevalue types
 * of options.
 *
 * Raw tokens are parsed and split around the '=' character. If none is found, it means
 * the option is of the following form : -DCOMMAND_LINE_DEFINE which represents a simple #define macro
 * If a '=' pivot character is found, the left part of the '=' is parsed as the #define macro symbol name, and
 * the right part of '=' is interpreted as the value conveyed by this option, using its std::string representation only.
 */
struct DefinitionOption : public CompilerOption
{
  /**
     * @brief determines whether the given token is part of the static map of available optimizations flags or not
     * @param[in]   _token : string representation of current flag being parsed
     * @return true : token exist in collection ; false : token is not part of the collection, thus it is not part of the available set of optimizations.
    */
  static bool can_create(const std::string& _token);

  /**
   * @brief Generates an Atmel Studio compatible representation of the embedded definition.
   * If a definition is passed like this : "-DTEST_DEFINITION=33", then the returned definition will be "TEST_DEFINITION=33"
   * @param atmel_studio_compat : essentially discarded
   * @return std::string
   */
  std::string generate(const bool atmel_studio_compat = true) override;

  /**
   * @brief Standard default constructor .
   */
  DefinitionOption();

  /**
   * @brief Builds a DefinitionOption based on a raw token parsed from command line.
   * Token is parsed at construction time in the aim to retrieve the name of the definition option
   * and its value, if one is provided with the -D<NAME>=<VALUE> pattern
   *
   * @param _token : raw token parsed from command line input
   */
  DefinitionOption(const std::string& _token);

  std::string defsymbol;    /**< Defined symbol with the stripped -D                                                                                   */
  std::string value;        /**< Payload of a given option, if any is given. e.g : "-DUSE_FEATURE=1" option gets the "-DUSE_FEATURE" token + "1" value */

private:
  /**
   * @brief provides some means to parse an incoming flag like "-DUSE_FEATURE=1" where the "=" is the pivot point
   * If no right part is found, the whole raw_token is used as the regular token (e.g. "-DUNIT_TESTING" does not
   * have a right part, so the final token is the same as the original one : token = "-DUNIT_TESTING")
   * @param raw_token : token parsed from command line input
  */
  void parse(const std::string& raw_token);
};

}
