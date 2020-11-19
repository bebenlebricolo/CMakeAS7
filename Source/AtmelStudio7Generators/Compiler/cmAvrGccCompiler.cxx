#include "cmAvrGccCompiler.h"

#include <algorithm>

#include "cmStringUtils.h"
#include "cmAvrGccWarningOption.h"
#include "cmAvrGccOptimizationOption.h"
#include "cmAvrGccLinkerOption.h"
#include "cmAvrGccDebugOption.h"
#include "cmAvrGccMachineOption.h"
#include "cmAvrGccDefinitionOption.h"

namespace compiler {

bool CompilerOptionFactory::is_valid(const std::string& token)
{
  if (token[0] != '-') {
    return false;
  }

  const std::string prefixes = "WfODgwEnm";
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
      out = std::make_shared<DefinitionOption>(token);
      break;

    // Debug flags
    case 'g':
      if (DebugOption::can_create(token)) {
        out = std::make_shared<DebugOption>(token);
      }
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

    // Machine-dependent options
    case 'm':
      out = std::make_shared<MachineOption>(token);
      break;

    // Generic, normal flags
    case 'f':
    case 'w':
    case 'n':
    default:
      out = std::make_shared<CompilerOption>(CompilerOption::Type::Generic, token);
      break;

  }

  return out;
}

void cmAvrGccCompiler::parse_flags(const std::string& flags)
{
  std::vector<std::string> tokens = cmutils::strings::split(flags);
  parse_flags(tokens);
}

const std::vector<cmAvrGccCompiler::ShrdOption>& cmAvrGccCompiler::get_options(const CompilerOption::Type type) const
{
  switch(type)
  {
    case CompilerOption::Type::Debug:
      return options.debug;
    case CompilerOption::Type::Definition:
      return options.definitions;
    case CompilerOption::Type::Linker:
      return options.linker;
    case CompilerOption::Type::Optimization:
      return options.optimizations;
    case CompilerOption::Type::Warning:
      return options.warnings;
    case CompilerOption::Type::Machine:
      return options.machine;
    case CompilerOption::Type::Generic:
    default:
      return options.normal;
  }
}

CompilerOption * cmAvrGccCompiler::get_option(const std::string& token) const
{
  ShrdOption opt = options.get_option(token, options.warnings);
  if (nullptr != opt) {
    return opt.get();
  }
  opt = options.get_option(token, options.debug);
  if (nullptr != opt) {
    return opt.get();
  }
  opt = options.get_option(token, options.definitions);
  if (nullptr != opt) {
    return opt.get();
  }
  opt = options.get_option(token, options.machine);
  if (nullptr != opt) {
    return opt.get();
  }
  opt = options.get_option(token, options.normal);
  if (nullptr != opt) {
    return opt.get();
  }
  opt = options.get_option(token, options.optimizations);
  if (nullptr != opt) {
    return opt.get();
  }
  opt = options.get_option(token, options.linker);
  if (nullptr != opt) {
    return opt.get();
  }
  return nullptr;
}


void cmAvrGccCompiler::parse_flags(const std::vector<std::string>& tokens)
{
  for (const auto& token : tokens) {
    if (compiler::CompilerOptionFactory::is_valid(token)) {
      ShrdOption flag = compiler::CompilerOptionFactory::create(token);
      options.accept_flag(flag);
    }
  }
}

void cmAvrGccCompiler::Options::push_flag(const ShrdOption& flag, OptionsVec& vec)
{
  if (!contains(flag->get_token(), vec)) {
    vec.push_back(flag);
  }
}


void cmAvrGccCompiler::Options::accept_flag(const ShrdOption& flag)
{
  if (flag == nullptr) {
    return;
  }

  switch (flag->get_type()) {
    case compiler::CompilerOption::Type::Optimization:
      push_flag(flag, optimizations);
      break;

    case compiler::CompilerOption::Type::Warning:
      push_flag(flag, warnings);
      break;

    case compiler::CompilerOption::Type::Linker:
      push_flag(flag, linker);
      break;

    case compiler::CompilerOption::Type::Debug:
      push_flag(flag, debug);
      break;

    case compiler::CompilerOption::Type::Definition:
      push_flag(flag, definitions);
      break;

    case compiler::CompilerOption::Type::Machine:
      push_flag(flag, machine);
      break;

    default:
      push_flag(flag, normal);
      break;
  }
}

bool cmAvrGccCompiler::has_option(const std::string& option) const
{
  return options.contains(option);
}

void cmAvrGccCompiler::clear()
{
  options.debug.clear();
  options.definitions.clear();
  options.warnings.clear();
  options.optimizations.clear();
  options.normal.clear();
  options.linker.clear();
  options.machine.clear();
}

bool cmAvrGccCompiler::Options::contains(const std::string& token) const
{
  if (contains(token, debug)) {
    return true;
  }
  if (contains(token, normal)) {
    return true;
  }
  if (contains(token, optimizations)) {
    return true;
  }
  if (contains(token, warnings)) {
    return true;
  }
  if (contains(token, linker)) {
    return true;
  }
  if (contains(token, machine)) {
    return true;
  }
  if (contains(token, definitions)) {
    return true;
  }
  return false;
}

bool cmAvrGccCompiler::Options::contains(const std::string& token, const OptionsVec& reference) const
{
  auto found_item = std::find_if(reference.begin(), reference.end(), [token](const ShrdOption& target)
  {
    return token == target->get_token();
  });

  return (found_item != reference.end());
}

cmAvrGccCompiler::ShrdOption cmAvrGccCompiler::Options::get_option(const std::string& token, const OptionsVec& vect) const
{
  const auto& found_item = std::find_if(vect.begin(), vect.end(), [token](const ShrdOption& opt)
  {
    return token == opt->get_token();
  });

  if (found_item != vect.end())
  {
    return *found_item;
  }
  return nullptr;
}

bool cmAvrGccCompiler::Options::is_unique(const std::string& token, const OptionsVec& reference) const
{

  unsigned int count = std::count_if(reference.begin(), reference.end(), [token](const ShrdOption& target)
  {
    return token == target->get_token();
  });

  return (count == 1);
}

bool cmAvrGccCompiler::Options::is_unique(const ShrdOption& option, const OptionsVec& reference) const
{

  unsigned int count = std::count_if(reference.begin(), reference.end(), [option](const ShrdOption& target)
  {
    return option == target;
  });

  return (count == 1);
}

}