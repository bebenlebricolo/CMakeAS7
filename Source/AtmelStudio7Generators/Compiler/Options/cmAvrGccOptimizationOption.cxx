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

#include "cmAvrGccOptimizationOption.h"
#include <algorithm>

namespace compiler {

std::unordered_map<OptimizationOption::Level, AS7OptionRepresentation> OptimizationOption::available_opt = {
  { OptimizationOption::Level::O0, { "-O0", "None (-O0)" } },
  { OptimizationOption::Level::O1, { "-O1", "Optimize(-O1)" } },
  { OptimizationOption::Level::O, { "-O", "Optimize(-O1)" } },
  { OptimizationOption::Level::O2, { "-O2", "Optimize more (-O2)" } },
  { OptimizationOption::Level::O3, { "-O3", "Optimize most (-O3)" } },
  { OptimizationOption::Level::Ofast, { "-Ofast", "Optimize most (-O3)" } },
  { OptimizationOption::Level::Og, { "-Og", "Optimize debugging experience (-Og)" } },
  { OptimizationOption::Level::Os, { "-Os", "Optimize for size (-Os)" } }
};

bool OptimizationOption::operator>(const OptimizationOption& other) const
{
  return static_cast<uint8_t>(optLevel) > static_cast<uint8_t>(other.optLevel);
}

bool OptimizationOption::operator<(const OptimizationOption& other) const
{
  return static_cast<uint8_t>(optLevel) < static_cast<uint8_t>(other.optLevel);
}

bool OptimizationOption::operator<=(const OptimizationOption& other) const
{
  return static_cast<uint8_t>(optLevel) <= static_cast<uint8_t>(other.optLevel);
}

bool OptimizationOption::operator>=(const OptimizationOption& other) const
{
  return static_cast<uint8_t>(optLevel) >= static_cast<uint8_t>(other.optLevel);
}

bool OptimizationOption::operator==(const OptimizationOption& other) const
{
  return static_cast<uint8_t>(optLevel) == static_cast<uint8_t>(other.optLevel);
}

bool OptimizationOption::operator!=(const OptimizationOption& other) const
{
  return static_cast<uint8_t>(optLevel) != static_cast<uint8_t>(other.optLevel);
}

OptimizationOption::Level OptimizationOption::get_level() const
{
  return optLevel;
}

std::pair<OptimizationOption::Level, AS7OptionRepresentation> OptimizationOption::get_default()
{
  auto& default_opt = available_opt[Level::Os];
  return {Level::Os, default_opt};
}

bool OptimizationOption::can_create(const std::string& _token)
{
  auto found_element = std::find_if(available_opt.begin(), available_opt.end(), [_token](const std::pair<Level, AS7OptionRepresentation>& element) {
    return _token == element.second.option;
  });
  return (found_element != available_opt.end());
}

std::pair<OptimizationOption::Level, AS7OptionRepresentation*> OptimizationOption::resolve(const std::string& flag) const
{
  std::pair<Level, AS7OptionRepresentation*> out = { Level::O0, nullptr };
  const auto found_element = std::find_if(available_opt.begin(), available_opt.end(), [flag](const std::pair<Level, AS7OptionRepresentation>& element) {
    return flag == element.second.option;
  });

  if (found_element != available_opt.end()) {
    out.first = found_element->first;
    out.second = &found_element->second;
  }

  return out;
}

OptimizationOption::OptimizationOption() : CompilerOption(Type::Optimization)
{}

OptimizationOption::OptimizationOption(const std::string& _token)
  : CompilerOption(Type::Optimization, _token)
{
  auto resolved_pair = resolve(_token);
  if (resolved_pair.second != nullptr) {
    CompilerOption::description = resolved_pair.second->atmel_studio_description;
    optLevel = resolved_pair.first;
  } else {
    CompilerOption::description = _token;
  }
}

std::string OptimizationOption::generate(const bool atmel_studio_compat)
{
  if (atmel_studio_compat) {
    return description;
  } else {
    return token;
  }
}

}
