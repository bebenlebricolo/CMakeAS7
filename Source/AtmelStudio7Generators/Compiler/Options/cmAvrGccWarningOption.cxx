#include "cmAvrGccWarningOption.h"
#include "cmStringUtils.h"

namespace compiler
{

bool WarningOption::can_create(const std::string& _token)
{
  return (!_token.empty()) && (_token[1] == 'W');
}

WarningOption::WarningOption()
    : CompilerOption(Type::Warning)
{}

WarningOption::WarningOption(const std::string& _token)
    : CompilerOption(Type::Warning, _token)
{}


}
