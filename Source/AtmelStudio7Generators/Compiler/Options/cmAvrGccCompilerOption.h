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

namespace compiler {

/**
 * @brief this structure packs data concerning atmel studio representation of an option
*/
struct AS7OptionRepresentation
{
  AS7OptionRepresentation() = default;
  AS7OptionRepresentation(const std::string& _opt,
                          const std::string& _desc);
  std::string option;                   /**< Basic option string (the one used by the compiler). e.g : option = "-ggdb" */
  std::string atmel_studio_description; /**< atmel studio 7 description of this option, if any                          */
};

/**
 * @brief this structure is used to represent basic options for AvrGcc compiler.
 * It uses the @see CompilerOption::Type enumerate to distinguish each possible kinds of
 * compiler options, which can have their own subtilities.
 */
struct CompilerOption
{
  /**
   * @brief describes the available kinds of CompilerOption.
   * Whenever special handling is required, a new class inherits from CompilerOption and
   * provides additional services according to the represented type
   */
  enum class Type
  {
    Generic,         /**< Used to represent basic options or flag triggered by the -f flag   */
    Optimization,    /**< Optimizations options are triggered by the -O flag                 */
    Debug,           /**< Debugging options are triggered by the -g flag                     */
    Warning,         /**< Warning options are triggered by the -W flag                       */
    Linker,          /**< Linker options are triggered by the -Wl flag                       */
    Definition,      /**< Definition options are triggered by the -D flag                    */
    Machine,         /**< Machine options are triggered by the -m flag                       */
    LanguageStandard /**< Language options are triggered by the -std flag                    */
  };

  /**
   * @brief Builds a CompilerOption using the given type only, all members are set to default values.
   * @param _type   :   underlying type of CompilerOption
   */
  CompilerOption(const Type _type);

  /**
   * @brief Builds a compiler option using the given token. Token is resolved and underlying type
   * is deduced using the previous token deduction.
   * @param _token : raw token parsed from command line
   */
  CompilerOption(const std::string& _token);

  /**
   * @brief Builds a compiler option using a token and a type information.
   * @param _type  : underlying type for which a new CompilerOption object will be built
   * @param _token : raw token parsed from command line
   */
  CompilerOption(const Type _type, const std::string& _token);

  /**
   * @brief Builds a compiler option using a raw token its description. Type information
   * will be deduced from the input raw token
   * @param _token       : raw token parsed from command line
   * @param _description : full description of this token.
   */
  CompilerOption(const std::string& _token, const std::string& _description);

  /**
   * @brief tells whether the given token is supported by the actual compiler model or not
   * @param _token  : input option/flag passed to the compiler as a command-line input
   * @return
   *    true  : option is supported and a representation can be created for it
   *    false : option is not supported, no representation can be created for it
  */
  static bool can_create(const std::string& _token);

  /**
     * @brief generates a string representation for this flag, which in the simplest cases is simply
     * the token used to build it (token = "-Wall" ; Generate() will return "-Wall").
     * We can as well choose to use the AtmelStudio representation, which modifies how the flag is represented :
     * for instance, "-Os" flag will be resolved as "Optimized for size ( -Os )" in Atmel Studio.
     * @param[in]   atmel_studio_compat :   generates a AtmelStudio-compatible version of this flag
     * @return generated string. Default value is the token value.
    */
  virtual std::string generate(const bool atmel_studio_compat = true);

  /**
   * @brief Implements a comparison operator for max element research
   *
   * @param other  : another compiler option against which to compare
   * @return true  : this structure is lesser than the other
   * @return false : this structur is greater or equal to the other
   */
  virtual bool operator<(const CompilerOption& other);

  /**
     * @brief returns the underlying type of this compiler flag, when polymorphism is used
     *        This is useful when another object creates a flag from a token and does not know what
     *        kind of flag it is. So using the created pointer GetType() method will help to select an appropriate
     *        location to store this newly created flag (for instance, if the flag is an optimization flag, it could
     *        appropriately be stored in a vector of optimization flags)
     * @return type.
    */
  Type get_type() const;

  /**
   * @brief returns the underlying constant token (copy of input option/flag)
   * @return underlying token
  */
  std::string get_token() const;

  /**
   * @brief Cheks if incoming token is recognized by current option.
   *
   * @param token : a token to be checked
   * @return true (contains) or false (does not contain)
   */
  virtual bool contains(const std::string& token) const;

  /**
   * @brief returns the underlying constant description
   * @return underlying description
  */
  std::string get_description() const;

protected:
  Type type = Type::Generic; /**< Keeps the type information to be used later on                       */
  std::string token;         /**< String representation of this flag (a.k.a flag token, e.g. "-Wall")  */
  std::string description;   /**< Textual description of the flag                                      */
};

// A simple namespace will do it, no need for any object instantiation
namespace CompilerOptionFactory {

/**
 * @brief Checks if the given token can be categorized in any of the available
 * types of compiler options.
 * @return true  : a match was found, a compiler option object can be constructed using this token
 *         false : no matching compiler option could be found for this token, no compiler option could be instantiated with it.
 */
bool is_valid(const std::string& token);

/**
 * @brief instantiates several CompilerOption * objects using the token as input.
 *
 * A list of CompilerOptions* is returned because under certain circumstances, some raw tokens
 * are under their concatenated form (e.g. "-Wl,--gc-sections,--relax" which packs several linker options
 * within a single token).
 * So the raw token is split using the adequate splitting rule for the deduced compiler option type
 * and tokens are then parsed individually.
 * Finally, one or several objects are instantiated using the previously split tokens
 * and wrapped within a generic shared_ptr<CompilerOption>.
 *
 * Note : each object in this vector uses polymorphism and we can also retrieve their
 * real class using their embedded CompilerOption::Type information with a simple static_cast
 */
std::vector<std::shared_ptr<CompilerOption>> create(const std::string& token);
};

}
