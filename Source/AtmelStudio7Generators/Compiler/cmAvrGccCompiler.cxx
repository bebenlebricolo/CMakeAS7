#include "cmAvrGccCompiler.h"

#include <algorithm>

#include "cmStringUtils.h"
#include "cmAvrGccWarningOption.h"
#include "cmAvrGccOptimizationOption.h"
#include "cmAvrGccLinkerOption.h"
#include "cmAvrGccDebugOption.h"

namespace compiler {

bool CompilerOptionFactory::is_valid(const std::string& token)
{
  if (token[0] != '-') {
    return false;
  }

  const std::string prefixes = "WfODg";
  // Check if second character of token is part of the above characters list
  return (prefixes.find(token[1]) != std::string::npos);
}

std::shared_ptr<CompilerOption> CompilerOptionFactory::create(const std::string& token)
{
  std::shared_ptr<CompilerOption> out;
  if (!is_valid(token)) {
    return out;
  }

  switch (token[1]) {
    case 'O':
      if (OptimizationOption::can_create(token)) {
        out = std::make_shared<OptimizationOption>(token);
      }
      break;

    // Compile definitions
    case 'D':
      out = std::make_shared<CompilerOption>(CompilerOption::Type::Definition, token);
      break;

    // Debug flags
    case 'g':
      out = std::make_shared<DebugOption>(token);
      break;

    // Generic, normal flags
    case 'f':
      out = std::make_shared<CompilerOption>(CompilerOption::Type::Generic, token);
      break;

    // Warning flags or linker flags (this is resolved with the help of the next character)
    case 'W':
      if (token[2] == 'l') {
        out = std::make_shared<LinkerOption>(token);
      }
      else
      {
        out = std::make_shared<WarningOption>(token);
      }
      break;

    default:
      break;
  }

  return out;
}

void cmAvrGccCompiler::parse_flags(const std::string& flags)
{
  std::vector<std::string> tokens = cmutils::strings::split(flags);
  parse_flags(tokens);
}

const std::vector<cmAvrGccCompiler::ShrdCompilerOption>& cmAvrGccCompiler::get_options(const CompilerOption::Type type) const
{
  switch(type)
  {
    case CompilerOption::Type::Debug:
      return debug;
    case CompilerOption::Type::Definition:
      return definitions;
    case CompilerOption::Type::Linker:
      return linker;
    case CompilerOption::Type::Optimization:
      return optimizations;
    case CompilerOption::Type::Warning:
      return warnings;
    case CompilerOption::Type::Generic:
    default:
      return normal;
  }
}

void cmAvrGccCompiler::parse_flags(const std::vector<std::string>& tokens)
{
  for (const auto& token : tokens) {
    if (compiler::CompilerOptionFactory::is_valid(token)) {
      std::shared_ptr<compiler::CompilerOption> flag = compiler::CompilerOptionFactory::create(token);
      accept_flag(flag);
    }
  }
}

void cmAvrGccCompiler::accept_flag(const ShrdCompilerOption& flag)
{
  if (flag == nullptr) {
    return;
  }

  switch (flag->GetType()) {
    case compiler::CompilerOption::Type::Optimization:
      if (is_unique(flag, optimizations)) {
        optimizations.push_back(flag);
      }
      break;

    case compiler::CompilerOption::Type::Warning:
      warnings.push_back(flag);
      break;

    case compiler::CompilerOption::Type::Linker:
      linker.push_back(flag);
      break;

    case compiler::CompilerOption::Type::Debug:
      debug.push_back(flag);
      break;

    case compiler::CompilerOption::Type::Definition:
      definitions.push_back(flag);
      break;

    default:
      normal.push_back(flag);
      break;
  }
}

bool cmAvrGccCompiler::contains(const std::string& token, const std::vector<ShrdCompilerOption>& reference) const
{
  auto found_item = std::find_if(reference.begin(), reference.end(), [token](const ShrdCompilerOption& target)
  {
    return token == target->get_token();
  });

  return (found_item != reference.end());
}

bool cmAvrGccCompiler::is_unique(const std::string& token, const std::vector<ShrdCompilerOption>& reference) const
{

  unsigned int count = std::count_if(reference.begin(), reference.end(), [token](const ShrdCompilerOption& target)
  {
    return token == target->get_token();
  });

  return (count == 1);
}

bool cmAvrGccCompiler::is_unique(const ShrdCompilerOption& option, const std::vector<ShrdCompilerOption>& reference) const
{

  unsigned int count = std::count_if(reference.begin(), reference.end(), [option](const ShrdCompilerOption& target)
  {
    return option == target;
  });

  return (count == 1);
}

}