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

std::shared_ptr<CompilerOption> CompilerFlagFactory::create(const std::string& token)
{
  std::shared_ptr<CompilerOption> out;
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
      out = std::make_shared<CompilerOption>(CompilerOption::Type::Definition, token);
      break;

    // Debug flags
    case 'g':
      out = std::make_shared<CompilerOption>(CompilerOption::Type::Debug, token);
      break;

    // Generic, normal flags
    case 'f':
      out = std::make_shared<CompilerOption>(CompilerOption::Type::Generic, token);
      break;

    // Warning flags or linker flags (this is resolved with the help of the next character)
    // TODO : implement linker flag vs warning flag resolution
    case 'W':
      if (token[2] == 'l') {
        out = std::make_shared<LinkerOption>(token)
      }
      out = std::make_shared<WarningOption>(token);
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

#ifdef UNIT_TESTING

const std::vector<cmAvrGccCompiler::FlagContainer>& cmAvrGccCompiler::get_optimization_flags() const
{
  return optimization_flags;
}

const std::vector<cmAvrGccCompiler::FlagContainer>& cmAvrGccCompiler::get_debug_flags() const
{
  return debug_flags;
}

const std::vector<cmAvrGccCompiler::FlagContainer>& cmAvrGccCompiler::get_warning_flags() const
{
  return warning_flags;
}

const std::vector<cmAvrGccCompiler::FlagContainer>& cmAvrGccCompiler::get_linker_flags() const
{
  return linker_flags;
}

const std::vector<cmAvrGccCompiler::FlagContainer>& cmAvrGccCompiler::get_normal_flags() const
{
  return normal_flags;
}

const std::vector<cmAvrGccCompiler::FlagContainer>& cmAvrGccCompiler::get_definitions() const
{
  return definitions;
}

#endif

void cmAvrGccCompiler::parse_flags(const std::vector<std::string>& tokens)
{
  for (const auto& token : tokens) {
    if (compiler::CompilerFlagFactory::is_valid(token)) {
      std::shared_ptr<compiler::CompilerOption> flag = compiler::CompilerFlagFactory::create(token);
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
    case compiler::CompilerOption::Type::Optimization:
      if (std::find(optimization_flags.begin(), optimization_flags.end(), flag) == optimization_flags.end()) {
        optimization_flags.push_back(flag);
      }
      break;

    case compiler::CompilerOption::Type::Warning:
      warning_flags.push_back(flag);
      break;

    case compiler::CompilerOption::Type::Debug:
      debug_flags.push_back(flag);
      break;

    case compiler::CompilerOption::Type::Definition:
      definitions.push_back(flag);
      break;

    default:
      normal_flags.push_back(flag);
      break;
  }
}

}