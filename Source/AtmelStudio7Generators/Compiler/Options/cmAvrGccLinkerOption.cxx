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

#include "cmAvrGccLinkerOption.h"
#include "cmStringUtils.h"

namespace compiler
{

bool LinkerOption::can_create(const std::string& _token)
{
  return (!_token.empty()) && (_token.find("-Wl") != std::string::npos);
}

LinkerOption::LinkerOption()
    : CompilerOption(Type::Linker)
{}

LinkerOption::LinkerOption(const std::string& _token)
    : CompilerOption(Type::Linker, _token)
{}

std::vector<std::string> LinkerOption::split_concatenated_options(const std::string& _token)
{
  std::vector<std::string> out = cmutils::strings::split(_token, ',');
  // remove first element as it contains only "Wl"
  out.erase(out.begin());

  // Add back "-Wl," prefix to each element and return the vector
  for (auto& elem : out) {
    elem = "-Wl," + elem;
  }
  return out;
}

bool LinkerOption::contains(const std::string& input_token) const
{
  auto split_result = cmutils::strings::split(input_token,',');
  auto split_option = cmutils::strings::split(this->token,',')[1];

  std::string naked_input_token = input_token;
  if (split_result.size() >= 2) {
    naked_input_token = split_result[1];
  }

  return split_option == naked_input_token;
}

}
