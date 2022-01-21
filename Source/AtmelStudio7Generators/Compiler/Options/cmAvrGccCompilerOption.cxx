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

#include "cmAvrGccCompilerOption.h"

namespace compiler
{

AS7OptionRepresentation::AS7OptionRepresentation(const std::string& _opt, const std::string& _desc) :
  option(_opt),
  atmel_studio_description(_desc)
{}


bool CompilerOption::can_create( const std::string& _token)
{
  (void)_token;
  return true;
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

// Not implemented
bool CompilerOption::operator<(const CompilerOption& other)
{
  (void) other;
  return false;
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

bool CompilerOption::contains(const std::string& input_token) const
{
  return input_token == token;
}

std::string CompilerOption::get_description() const
{
  return description;
}




} /* !compiler */