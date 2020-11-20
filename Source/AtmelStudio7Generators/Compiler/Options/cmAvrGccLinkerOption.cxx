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


}
