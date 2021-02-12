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

#include "cmAvrGccMachineOption.h"
#include "cmStringUtils.h"

namespace compiler
{

// see https://linux.die.net/man/1/avr-gcc
std::unordered_set<std::string> MachineOption::available_flags =
{
  "-mmcu",
  "-mcall-prologues",
  "-mno-interrupts",
  "-mtiny-stack",
  "-mrelax",
  "-mint8"
};

bool MachineOption::can_create(const std::string& _token)
{
  const auto vec = cmutils::strings::split(_token, '=');
  const auto& found_it = available_flags.find(vec[0]);
  return (found_it != available_flags.end());
}


MachineOption::MachineOption()
    : CompilerOption(Type::Machine)
{ }

MachineOption::MachineOption(const std::string& _token)
    : CompilerOption(Type::Machine)
{
  parse(_token);
}

void MachineOption::parse(const std::string& raw_token)
{
  std::vector<std::string> split = cmutils::strings::split(raw_token, '=');
  token = split[0];

  if (split.size() != 1)
  {
    value = split[1];
  }
}


}
