#include "cmAvrGccCompilerOption.h"

namespace compiler
{

AS7OptionRepresentation::AS7OptionRepresentation(const std::string& _opt, const std::string& _desc) :
  option(_opt),
  atmel_studio_description(_desc)
{}


bool CompilerOption::can_create( const std::string& _token)
{
  return false;
}

CompilerOption::CompilerOption(const Type _type)
  : type(_type)
{
}

CompilerOption::CompilerOption(const std::string& _token)
    : token(_token)
{
}

CompilerOption::CompilerOption(const Type _type, const std::string& _token)
    : type(_type)
    , token(_token)
{
}

CompilerOption::CompilerOption(const std::string& _token, const std::string& _description)
    : token(_token)
    , description(_description)
{
}

std::string CompilerOption::generate(const bool atmel_studio_compat)
{
  if (atmel_studio_compat) {
    return description;
  }
  return token;
}

CompilerOption::Type CompilerOption::get_type() const
{
  return type;
}

std::string CompilerOption::get_token() const
{
  return token;
}

std::string CompilerOption::get_description() const
{
  return description;
}




} /* !compiler */