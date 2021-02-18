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

// TODO : this could be moved to a more generic place for better language handling accross all AtmelStudio7Generators project
/**
 * @brief depicts some Language abstractions used to resolve the targeted language
 * from its string representation or to be able to process language standards later on.
 */
struct Language
{
  /**
   * @brief gives a list of supported languages.
   */
  enum class Lang
  {
    Undefined, /**< Default value used as a fallback    */
    C,         /**< C language                          */
    CXX        /**< C++ language                        */
  };

  /**
   * @brief Instanciates a default container for the language representation.
   * Shall be updated manually later on (no method will work with Undefined language settings)
   */
  Language()
    : lang(Lang::Undefined)
  {
  }

  /**
   * @brief Instanciates a default container for the language representation.
   * @param l : input language
   */
  Language(const Lang l)
    : lang(l)
  {
  }

  /**
   * @brief Standard Copy constructor.
   * @param other : other language object
   */
  Language(const Language& other)
    : lang(other.lang)
  {
  }

  /**
   * @brief Standard assignment operator.
   *
   * @param other : other Language object
   * @return itself
   */
  Language& operator=(const Language& other)
  {
    lang = other.lang;
    return *this;
  }

  /**
   * @brief Standard assignment operator using the enum value of Language instead
   * of the complete object
   *
   * @param other : targeted language
   * @return itself
   */
  Language& operator=(const Language::Lang& _lang)
  {
    lang = _lang;
    return *this;
  }

  /**
   * @brief returns a std::string representation of this language object.
   */
  std::string to_string()
  {
    switch (lang) {
      case Lang::C:
        return "C";

      case Lang::CXX:
        return "CXX";

      case Lang::Undefined:
      default:
        return "Undefined";
    }
  }

private:
  Lang lang; /**< Embedded enumerate used for comparison purposes */
};

/**
 * @brief Language standard option is used to interprete language standards such as -std=c++17 for instance.
 * This structure provides services to parse all known standards, which could either come from
 * ISO, gnu or shorthand C++ standards.
 */
struct LanguageStandardOption : public CompilerOption
{

  /**
     * @brief determines whether the given token is part of the static map of available optimizations flags or not
     * @param[in]   _token : string representation of current flag being parsed
     * @return true : token exist in collection ; false : token is not part of the collection, thus it is not part of the available set of optimizations.
    */
  static bool can_create(const std::string& _token);

  /**
   * @brief Standard constructor for LanguageStandardOption.
   */
  LanguageStandardOption();

  /**
   * @brief Builds a LanguageStandardOption using the given raw token to deduce the right
   * standard representation for this token.
   *
   * @param _token : raw token parsed from command line input
   */
  LanguageStandardOption(const std::string& _token);

  std::string value; /**< Payload of a given option, if any is given. e.g : "-mmcu=atmega328p" option gets the "-mmcu" token + "atmega328p" value */
  Language lang;     /**< Language to which this standard applies                                                                                 */

private:
  static std::vector<std::string> c_standards;    /**< Lists all available C standards keys */
  static std::vector<std::string> cpp_standards;  /**< Lists all available C++ standards keys */

  /**
   * @brief Checks that a token exists in the referenced vector.
   *
   * @param ref     : reference vector
   * @param token   : raw token
   * @return true  : token is contained in reference vector
   *         false : token does not appear within reference vector
   */
  static bool exists_in(const std::vector<std::string>& ref, std::string token);

  /**
   * @brief provides some means to parse an incoming flag like "-mmcu=atmega328p" where the "=" is the pivot point
   * If no right part is found, the whole raw_token is used as the regular token (e.g. "-mcall-prologues" does not
   * have a right part, so the final token is the same as the original one : token = "-mcall-prologues")
   * @param raw_token : token parsed from command line input
  */
  void parse(const std::string& raw_token);
};

}
