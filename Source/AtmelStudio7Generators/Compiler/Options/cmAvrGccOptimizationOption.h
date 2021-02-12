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
#include <unordered_map>
#include <vector>

#include "cmAvrGccCompilerOption.h"

namespace compiler {

/**
 * @brief Provides services to identify optimizations opions passed to avr-gcc using the "-O" flag.
 * An AtmelStudio7 compatible representation for those options which could be used while generating
 * AtmelStudio7's projet files.
 */
struct OptimizationOption : public CompilerOption
{
  /**
     * @brief packs all optimization levels available in avr-gcc.
     * Note that options are ordered in a way which allows sorting of many
     * OptimizationOptions later on.
     * When multiples OptimizationOptions are given to the compiler, the highest ranking one
     * will be retrieved. For instance, if "-O3 -O0" are given, the maximum element in this sequence
     * is "-O0" based on gcc precedence rules.
     */
  enum class Level : uint8_t
  {
    O1,    /**< Optimization for code size and execution time          */
    O,     /**< Equivalent of O1                                       */
    O2,    /**< Optimize more for code size and execution time         */
    O3,    /**< Optimize more for code size and execution time         */
    Og,    /**< Optimize but keep enough information to help debugging */
    Os,    /**< Optimize for code size                                 */
    Ofast, /**< O3 with fast none accurate math calculations           */
    O0,    /**< No Optimizations whatsoever, prevails on other options */
  };

  /**
     * @brief Returns the default optimization settings for atmelstudio
    */
  static std::pair<Level, AS7OptionRepresentation> get_default();

  /**
     * @brief determines whether the given token is part of the static map of available optimizations flags or not
     * @param[in]   _token : string representation of current flag being parsed
     * @return true : token exist in collection ; false : token is not part of the collection, thus it is not part of the available set of optimizations.
    */
  static bool can_create(const std::string& _token);

  /**
     * @brief Standard default constructor.
     */
  OptimizationOption();

  /**
     * @brief Builds an OptimizationOption using given token to resolve the right
     * Optimization representation.
     * @param _token : raw token as parsed from command line input
     */
  OptimizationOption(const std::string& _token);

  /**
    * @brief > operator is overloaded to allow sorting of OptimizationOption(s) based on
    * their inner debugging level. Comparison is operated on the optLevel member.
    *
    * @param other  : other OptimizationOption object
    * @return true  : this->optLevel is > other.optLevel
    *         false : this->optLevel is <= other.optLevel
    */
  bool operator>(const OptimizationOption& other) const;

  /**
    * @brief < operator is overloaded to allow sorting of OptimizationOption(s) based on
    * their inner debugging level. Comparison is operated on the optLevel member.
    *
    * @param other  : other OptimizationOption object
    * @return true  : this->optLevel is < other.optLevel
    *         false : this->optLevel is >= other.optLevel
    */
  bool operator<(const OptimizationOption& other) const;

  /**
    * @brief <= operator is overloaded to allow sorting of OptimizationOption(s) based on
    * their inner debugging level. Comparison is operated on the optLevel member.
    *
    * @param other  : other OptimizationOption object
    * @return true  : this->optLevel is <= other.optLevel
    *         false : this->optLevel is > other.optLevel
    */
  bool operator<=(const OptimizationOption& other) const;

  /**
    * @brief >= operator is overloaded to allow sorting of OptimizationOption(s) based on
    * their inner debugging level. Comparison is operated on the optLevel member.
    *
    * @param other  : other OptimizationOption object
    * @return true  : this->optLevel is >= other.optLevel
    *         false : this->optLevel is < other.optLevel
    */
  bool operator>=(const OptimizationOption& other) const;

  /**
    * @brief == operator is overloaded to allow sorting of OptimizationOption(s) based on
    * their inner debugging level. Comparison is operated on the optLevel member.
    *
    * @param other  : other OptimizationOption object
    * @return true  : this->optLevel is == other.optLevel
    *         false : this->optLevel is != other.optLevel
    */
  bool operator==(const OptimizationOption& other) const;

  /**
    * @brief != operator is overloaded to allow sorting of OptimizationOption(s) based on
    * their inner debugging level. Comparison is operated on the optLevel member.
    *
    * @param other  : other OptimizationOption object
    * @return true  : this->optLevel is != other.optLevel
    *         false : this->optLevel is == other.optLevel
    */
  bool operator!=(const OptimizationOption& other) const;

  /**
   * @brief Retrieves internally stored Optimization level.
   * @return Level
   */
  Level get_level() const;

  /**
   * @brief generates a string representation of this option using (or not) an
   * AS7 compatible formalism.
   *
   * @param atmel_studio_compat : activates AS7 compatibility or not
   */
  std::string generate(const bool atmel_studio_compat = true) override;

private:
  static std::unordered_map<Level, AS7OptionRepresentation> available_opt;  /**< List of all available options */

  /**
   * @brief Use raw input token to resolve the right pair of Level/AS7OptionRepresentation located
   * within the available_opt map.
   *
   * @param token : raw token as parsed from command line input
   */
  std::pair<Level, AS7OptionRepresentation*> resolve(const std::string& token) const;
  Level optLevel = Level::O0; /**< Each instance embeds its own optimization Level representation*/
};

}
