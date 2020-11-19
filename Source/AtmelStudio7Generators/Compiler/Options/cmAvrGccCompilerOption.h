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
  std::string option;                    /**< Basic option string (the one used by the compiler). e.g : option = "-ggdb" */
  std::string atmel_studio_description;  /**< atmel studio 7 description of this option, if any                          */
};

struct CompilerOption
{
  enum class Type
  {
    Generic,
    Optimization,
    Debug,
    Warning,
    Linker,
    Definition,
    Machine
  };

  CompilerOption(const Type _type);
  CompilerOption(const std::string& _token);
  CompilerOption(const Type _type, const std::string& _token);
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
   * @brief returns the underlying constant description
   * @return underlying description
  */
  std::string get_description() const;

protected:
  Type type = Type::Generic;  /**< Keeps the type information to be used later on                       */
  std::string token;          /**< String representation of this flag (a.k.a flag token, e.g. "-Wall")  */
  std::string description;    /**< Textual description of the flag                                      */
};

// A simple namespace will do it, no need for any object instantiation
namespace CompilerOptionFactory {
  bool is_valid(const std::string& token);
  std::shared_ptr<CompilerOption> create(const std::string& token);
};

}
