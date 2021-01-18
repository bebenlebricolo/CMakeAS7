#include "cmAvrGccDebugOption.h"
#include <algorithm>

namespace compiler
{

std::unordered_map<DebugOption::Level, AS7OptionRepresentation> DebugOption::available_options =
{
  {Level::None,   {"", "None"}},
  {Level::g,      {"-g", "Minimal (-g1)"}},
  {Level::g1,     {"-g1", "Minimal (-g1)"}},
  {Level::g2,     {"-g2", "Default (-g2)"}},
  {Level::g3,     {"-g3", "Maximum (-g3)"}},
  {Level::gdwarf, {"-gdwarf", "Default (-g2)"}},
  {Level::ggdb,   {"-ggdb", "Default (-g2)"}},
};

bool DebugOption::can_create(const std::string& _token)
{
  auto found_element = std::find_if(available_options.begin(),
                                    available_options.end(),
                                    [_token](const std::pair<Level, AS7OptionRepresentation>& element)
  {
    return _token == element.second.option;
  });
  return (found_element != available_options.end());
}

bool DebugOption::operator==(const DebugOption& other) const
{
  return optLevel == other.optLevel;
}

bool DebugOption::operator<=(const DebugOption& other) const
{
  return optLevel <= other.optLevel;
}

bool DebugOption::operator>=(const DebugOption& other) const
{
  return optLevel >= other.optLevel;
}

bool DebugOption::operator>(const DebugOption& other) const
{
  return optLevel > other.optLevel;
}

bool DebugOption::operator<(const DebugOption& other) const
{
  return optLevel < other.optLevel;
}

std::string DebugOption::generate(const bool atmel_studio_compat)
{
  if (atmel_studio_compat)
  {
    return description;
  }
  return token;
}

std::pair<DebugOption::Level, AS7OptionRepresentation*> DebugOption::resolve(const std::string& flag) const
{
  std::pair<Level, AS7OptionRepresentation*> out = { Level::None, nullptr };
  const auto found_element = std::find_if(available_options.begin(), available_options.end(), [flag](const std::pair<Level, AS7OptionRepresentation>& element) {
    return flag == element.second.option;
  });

  if (found_element != available_options.end()) {
    out.first = found_element->first;
    out.second = &found_element->second;
  }

  return out;
}

std::pair<DebugOption::Level, AS7OptionRepresentation> DebugOption::get_default()
{
  auto& default_opt = available_options[Level::None];
  return {Level::None, default_opt};
}

DebugOption::DebugOption()
  : CompilerOption(Type::Debug)
{
}

DebugOption::DebugOption(const std::string& _token)
  : CompilerOption(Type::Debug, _token)
{
  auto resolved_pair = resolve(_token);
  if (resolved_pair.second != nullptr) {
    CompilerOption::description = resolved_pair.second->atmel_studio_description;
    optLevel = resolved_pair.first;
  } else {
    CompilerOption::description = _token;
  }
}


}