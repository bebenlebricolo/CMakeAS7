#include "cmAvrGccCompiler.h"

#include "cmAvrGccOptimizationFlag.h"
#include "cmAvrGccWarningFlag.h"
#include "cmStringUtils.h"

namespace compiler {

bool CompilerFlagFactory::is_valid(const std::string& token)
{
  if (token[0] != '-') {
    return false;
  }

  const std::string prefixes = "WfOD";
  // Check if second character of token is part of the above characters list
  return (prefixes.find(token[1]) != std::string::npos);
}

std::shared_ptr<CompilerFlag> CompilerFlagFactory::create(const std::string& token)
{
  std::shared_ptr<CompilerFlag> out;
  if (!is_valid(token)) {
    return out;
  }

  switch (token[1]) {
    case 'O':
      if (OptimizationFlag::can_create(token)) {
        out = std::make_shared<OptimizationFlag>(token);
      }
      break;

    // Compile definitions
    case 'D':
      out = std::make_shared<CompilerFlag>(CompilerFlag::Type::Definition, token);
      break;

    // Debug flags
    case 'g':
      out = std::make_shared<CompilerFlag>(CompilerFlag::Type::Debug, token);
      break;

    // Generic, normal flags
    case 'f':
      out = std::make_shared<CompilerFlag>(CompilerFlag::Type::Generic, token);
      break;

    // Warning flags or linker flags (this is resolved with the help of the next character)
    // TODO : implement linker flag vs warning flag resolution
    case 'W':
      out = std::make_shared<WarningFlag>(token);
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

void cmAvrGccCompiler::parse_flags(const std::vector<std::string>& tokens)
{
  for (const auto& token : tokens) {
    if (compiler::CompilerFlagFactory::is_valid(token)) {
      std::shared_ptr<compiler::CompilerFlag> flag = compiler::CompilerFlagFactory::create(token);
      accept_flag(flag);
    }
  }
}

void cmAvrGccCompiler::accept_flag(const FlagContainer& flag)
{
  if (flag == nullptr) {
    return;
  }

  switch (flag->GetType()) {
    case compiler::CompilerFlag::Type::Optimization:
      if (std::find(optimization_flags.begin(), optimization_flags.end(), flag) == optimization_flags.end()) {
        optimization_flags.push_back(flag);
      }
      break;

    case compiler::CompilerFlag::Type::Warning:
      warning_flags.push_back(flag);
      break;

    case compiler::CompilerFlag::Type::Debug:
      debug_flags.push_back(flag);
      break;

    case compiler::CompilerFlag::Type::Definition:
      definitions.push_back(flag);
      break;

    default:
      normal_flags.push_back(flag);
      break;
  }
}

}