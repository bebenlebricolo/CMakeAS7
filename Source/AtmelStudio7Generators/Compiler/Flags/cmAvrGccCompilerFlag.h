#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace compiler {

struct CompilerFlag
{
  enum class Type
  {
    Generic,
    Optimization,
    Warning,
    Linker,
    Feature,
    Definition
  };

  static bool can_create(const std::string& _token) { return false; }

  CompilerFlag(const Type _type)
    : type(_type)
  {
  }
  CompilerFlag(const std::string& _token)
    : token(_token)
  {
  }
  CompilerFlag(const Type _type, const std::string& _token)
    : type(_type)
    , token(_token)
  {
  }
  CompilerFlag(const std::string& _token, const std::string& _description)
    : token(_token)
    , description(_description)
  {
  }

  /**
     * @brief generates a string representation for this flag, which in the simplest cases is simply
     * the token used to build it (token = "-Wall" ; Generate() will return "-Wall").
     * We can as well choose to use the AtmelStudio representation, which modifies how the flag is represented :
     * for instance, "-Os" flag will be resolved as "Optimized for size ( -Os )" in Atmel Studio.
     * @param[in]   atmel_studio_compat :   generates a AtmelStudio-compatible version of this flag
     * @return generated string. Default value is the token value.
    */
  virtual std::string Generate(const bool atmel_studio_compat = true) = 0;

  /**
     * @brief returns the underlying type of this compiler flag, when polymorphism is used
     *        This is useful when another object creates a flag from a token and does not know what
     *        kind of flag it is. So using the created pointer GetType() method will help to select an appropriate
     *        location to store this newly created flag (for instance, if the flag is an optimization flag, it could
     *        appropriately be stored in a vector of optimization flags)
     * @return type.
    */
  Type GetType() const { return type; }

protected:
  Type type = Type::Generic;
  std::string token;       /**< String representation of this flag (a.k.a flag token, e.g. "-Wall")        */
  std::string description; /**< Textual description of the flag                                            */
};

// A simple namespace will do it, no need for any object instantiation
namespace CompilerFlagFactory {
bool is_valid(const std::string& token);
std::shared_ptr<CompilerFlag*> create(const std::string& token);
};

}
