#include "cmAvrGccDefinitionOption.h"
#include "cmStringUtils.h"

namespace compiler
{

bool DefinitionOption::can_create(const std::string& _token)
{
  return (!_token.empty()) && (_token[1] == 'D');
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
  std::vector<std::string> splitted = cmutils::strings::split(raw_token, '=');
  defsymbol = splitted[0];

  // Strip the "-D"
  defsymbol = defsymbol.substr(2);

  if (splitted.size() != 1)
  {
    value = splitted[1];
  }
}


}
