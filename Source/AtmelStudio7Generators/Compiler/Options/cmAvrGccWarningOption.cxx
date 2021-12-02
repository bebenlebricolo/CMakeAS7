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

#include "cmAvrGccWarningOption.h"
#include "cmStringUtils.h"

namespace compiler
{

bool WarningOption::can_create(const std::string& _token)
{
  return (!_token.empty()) &&
          ((_token[1] == 'W') || (_token[1] == 'p'));
}

WarningOption::WarningOption()
    : CompilerOption(Type::Warning)
{}

WarningOption::WarningOption(const std::string& _token)
    : CompilerOption(Type::Warning, _token)
{}


}
