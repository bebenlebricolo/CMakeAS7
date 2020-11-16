#pragma once

#include "cmAvrGccOptimizationFlag.h"

namespace compiler {

std::unordered_map<OptimizationFlag::Level, OptimizationFlag::FullDescription> OptimizationFlag::available_opt = {
  { OptimizationFlag::Level::O0, { "-O0", "None (-O0)" } },
  { OptimizationFlag::Level::O1, { "-O1", "Optimize(-O1)" } },
  { OptimizationFlag::Level::O, { "-O", "Optimize(-O1)" } },
  { OptimizationFlag::Level::O2, { "-O2", "Optimize more (-O2)" } },
  { OptimizationFlag::Level::O3, { "-O3", "Optimize more (-O3)" } },
  { OptimizationFlag::Level::Ofast, { "-Ofast", "Optimize more (-O3)" } },
  { OptimizationFlag::Level::Og, { "-Og", "Optimize debugging experience (-Og)" } },
  { OptimizationFlag::Level::Os, { "-Os", "Optimize for size (-Os)" } }
};

bool OptimizationFlag::operator>(const OptimizationFlag& other) const
{
  return static_cast<uint8_t>(optLevel) > static_cast<uint8_t>(other.optLevel);
}

bool OptimizationFlag::operator<(const OptimizationFlag& other) const
{
  return static_cast<uint8_t>(optLevel) < static_cast<uint8_t>(other.optLevel);
}

bool OptimizationFlag::operator<=(const OptimizationFlag& other) const
{
  return static_cast<uint8_t>(optLevel) <= static_cast<uint8_t>(other.optLevel);
}

bool OptimizationFlag::operator>=(const OptimizationFlag& other) const
{
  return static_cast<uint8_t>(optLevel) >= static_cast<uint8_t>(other.optLevel);
}

bool OptimizationFlag::operator==(const OptimizationFlag& other) const
{
  return static_cast<uint8_t>(optLevel) == static_cast<uint8_t>(other.optLevel);
}

bool OptimizationFlag::operator!=(const OptimizationFlag& other) const
{
  return static_cast<uint8_t>(optLevel) != static_cast<uint8_t>(other.optLevel);
}

OptimizationFlag::Level OptimizationFlag::get_level() const
{
  return optLevel;
}

bool OptimizationFlag::can_create(const std::string& _token)
{
  auto found_element = std::find_if(available_opt.begin(), available_opt.end(), [_token](const std::pair<Level, FullDescription>& element) {
    return _token == element.second.flag;
  });
  return (found_element != available_opt.end());
}

std::pair<OptimizationFlag::Level, OptimizationFlag::FullDescription*> OptimizationFlag::resolve(const std::string& flag) const
{
  std::pair<Level, FullDescription*> out = { Level::O0, nullptr };
  const auto found_element = std::find_if(available_opt.begin(), available_opt.end(), [flag](const std::pair<Level, FullDescription>& element) {
    return flag == element.second.flag;
  });

  if (found_element != available_opt.end()) {
    out.first = found_element->first;
    out.second = &found_element->second;
  }

  return out;
}

OptimizationFlag::OptimizationFlag(const std::string& _token)
  : CompilerFlag(Type::Optimization, _token)
{
  auto resolved_pair = resolve(_token);
  if (resolved_pair.second != nullptr) {
    CompilerFlag::description = resolved_pair.second->atmel_studio_description;
    optLevel = resolved_pair.first;
  } else {
    CompilerFlag::description = _token;
  }
}

std::string OptimizationFlag::Generate(const bool atmel_studio_compat)
{
  if (atmel_studio_compat) {
    return description;
  } else {
    return token;
  }
}

}
