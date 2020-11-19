#pragma once

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include "cmAvrGccCompilerOption.h"

namespace compiler {

struct DefinitionOption : public CompilerOption
{
  /**
     * @brief determines whether the given token is part of the static map of available optimizations flags or not
     * @param[in]   _token : string representation of current flag being parsed
     * @return true : token exist in collection ; false : token is not part of the collection, thus it is not part of the available set of optimizations.
    */
  static bool can_create(const std::string& _token);

  DefinitionOption();
  DefinitionOption(const std::string& _token);

  std::string defsymbol;    /**< Defined symbol */
  std::string value;        /**< Payload of a given option, if any is given. e.g : "-mmcu=atmega328p" option gets the "-mmcu" token + "atmega328p" value */

private:
  /**
   * @brief provides some means to parse an incoming flag like "-mmcu=atmega328p" where the "=" is the pivot point
   * If no right part is found, the whole raw_token is used as the regular token (e.g. "-mcall-prologues" does not
   * have a right part, so the final token is the same as the original one : token = "-mcall-prologues")
   * @param raw_token : token parsed from command line input
  */
  void parse(const std::string& raw_token);
};

}
