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
 * @brief describes debugging options triggered by the  -g flag.
 */
struct DebugOption : public CompilerOption
{
  /**
   * @brief lists all available levels which could be interepreted from
   * command line.
   *
   * Note : priorities are applied in case where several flags are given at compile time
   * to ensure option's uniqueness.
   * For instance, when provided with -g1 and -ggdb options, -ggdb option has a
   * higher ranking in the enum range than -g1, so -ggdb will be the retained option when
   * data is being sorted and filtered to get the max element
   */
  enum class Level : uint8_t
  {
    None,   /**< no flags */
    g,      /**< -g       */
    g1,     /**< -g1      */
    g2,     /**< -g2      */
    g3,     /**< -g3      */
    ggdb,   /**< -ggdb    */
    gdwarf, /**< -gdwarf  */
  };

  /**
    * @brief determines whether the given token is part of the static map of available optimizations flags or not
    * @param[in]   _token : string representation of current flag being parsed
    * @return true : token exist in collection ; false : token is not part of the collection, thus it is not part of the available set of optimizations.
    */
  static bool can_create(const std::string& _token);

  /**
   * @brief returns the default value for debug flags
   * @return a pair of level and its corresponding representation
  */
  static std::pair<Level, AS7OptionRepresentation> get_default();

  /**
   * @brief Standard default constructor for DebugOption.
   */
  DebugOption();

  /**
   * @brief Builds a DebugOption object using the given raw token as an input.
   * @param _token : raw token parsed from command line
   */
  DebugOption(const std::string& _token);

  /**
   * @brief == operator is overloaded to allow sorting of DebugOption(s) based on
   * their inner debugging level. Comparison is operated on the optLevel member.
   *
   * @param other  : other DebugOption object
   * @return true  : this->optLevel == other.optLevel
   *         false : this->optLevel != other.optLevel
   */
  bool operator==(const DebugOption& other) const;

  /**
   * @brief >= operator is overloaded to allow sorting of DebugOption(s) based on
   * their inner debugging level. Comparison is operated on the optLevel member.
   *
   * @param other   : other DebugOption object
   * @return true  : this->optLevel is >= other.optLevel
   *         false : this->optLevel is < other.optLevel
   */
  bool operator>=(const DebugOption& other) const;

  /**
   * @brief <= operator is overloaded to allow sorting of DebugOption(s) based on
   * their inner debugging level. Comparison is operated on the optLevel member.
   *
   * @param other   : other DebugOption object
   * @return true  : this->optLevel is <= other.optLevel
   *         false : this->optLevel is > other.optLevel
   */
  bool operator<=(const DebugOption& other) const;

  /**
   * @brief > operator is overloaded to allow sorting of DebugOption(s) based on
   * their inner debugging level. Comparison is operated on the optLevel member.
   *
   * @param other   : other DebugOption object
   * @return true  : this->optLevel is > other.optLevel
   *         false : this->optLevel is <= other.optLevel
   */
  bool operator>(const DebugOption& other) const;

  /**
   * @brief < operator is overloaded to allow sorting of DebugOption(s) based on
   * their inner debugging level. Comparison is operated on the optLevel member.
   *
   * @param other   : other DebugOption object
   * @return true  : this->optLevel is < other.optLevel
   *         false : this->optLevel is >= other.optLevel
   */
  bool operator<(const DebugOption& other) const;

  /**
   * @brief generates a string representation of this debugging option.
   *
   * Depending on the atmel studio compatibility (or not), a special representation is given
   * instead of the the raw token.
   * If atmel studio compatibility is set to true : returns this->description
   * otherwise , return this->token
   *
   * @param atmel_studio_compat : flag used to generate an adequate AtmelStudio7 representation for this flag
   * @return the adequate string representing this option
   */
  std::string generate(const bool atmel_studio_compat = true) override;

private:
  static std::unordered_map<Level, AS7OptionRepresentation> available_options;       /**< Statically stores a map to the available AtmelStudio7 representation of each level*/

  /**
   * @brief resolves the adequate pair of level/AS7OptionRepresentation based on input raw token.
   * @param token   :   input raw token parsed from command line
   * @return pair of Level/AS7OptionRepresentation* or a default pair if token cannot be interpreted
   */
  std::pair<Level, AS7OptionRepresentation*> resolve(const std::string& token) const;
  Level optLevel = Level::None; /**< Embedded debugging option level information*/
};

}
