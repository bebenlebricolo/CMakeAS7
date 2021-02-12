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
 * @brief MachineOption structure provides services to parse options starting with -m flag
 * such as the -mmcu option (used to support AVR cores).
 */
struct MachineOption : public CompilerOption
{
  /**
     * @brief determines whether the given token is part of the static map of available optimizations flags or not
     * @param[in]   _token : string representation of current flag being parsed
     * @return true : token exist in collection ; false : token is not part of the collection, thus it is not part of the available set of optimizations.
    */
  static bool can_create(const std::string& _token);

  /**
   * @brief Standard Default constructor.
   */
  MachineOption();

  /**
   * @brief Builds a MachineOption using the input raw token. Token is parsed in order to deduce
   * the right category for the given option
   * @param _token : raw token parsed from command line input
   */
  MachineOption(const std::string& _token);

  std::string value;  /**< Payload of a given option, if any is given. e.g : "-mmcu=atmega328p" option gets the "-mmcu" token + "atmega328p" value */

private:
  static std::unordered_set<std::string> available_flags; /**< Set of all known available options from avr-gcc compiler*/

  /**
   * @brief provides some means to parse an incoming flag like "-mmcu=atmega328p" where the "=" is the pivot point
   * If no right part is found, the whole raw_token is used as the regular token (e.g. "-mcall-prologues" does not
   * have a right part, so the final token is the same as the original one : token = "-mcall-prologues")
   * @param raw_token : token parsed from command line input
  */
  void parse(const std::string& raw_token);
};

}
