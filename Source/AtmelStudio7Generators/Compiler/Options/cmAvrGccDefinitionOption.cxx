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

#include "cmAvrGccDefinitionOption.h"
#include "cmStringUtils.h"

namespace compiler
{

bool DefinitionOption::can_create(const std::string& _token)
{
  return (!_token.empty()) && (_token[1] == 'D');
}

std::string DefinitionOption::generate(const bool atmel_studio_compat)
{
  (void) atmel_studio_compat;
  return defsymbol + "=" + value;
}



DefinitionOption::DefinitionOption()
    : CompilerOption(Type::Definition)
{ }

DefinitionOption::DefinitionOption(const std::string& _token)
    : CompilerOption(Type::Definition, _token)
{
  parse(_token);
}

void DefinitionOption::parse(const std::string& raw_token)
{
  std::vector<std::string> split = cmutils::strings::split(raw_token, '=');
  defsymbol = split[0];

  // Strip the "-D"
  defsymbol = defsymbol.substr(2);

  if (split.size() != 1)
  {
    value = split[1];
  }
}


}
