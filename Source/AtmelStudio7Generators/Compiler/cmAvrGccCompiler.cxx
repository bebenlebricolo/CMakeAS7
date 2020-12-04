#include "cmAvrGccCompiler.h"

#include <algorithm>

#include "cmStringUtils.h"
#include "cmAvrGccWarningOption.h"
#include "cmAvrGccOptimizationOption.h"
#include "cmAvrGccLinkerOption.h"
#include "cmAvrGccDebugOption.h"
#include "cmAvrGccMachineOption.h"
#include "cmAvrGccDefinitionOption.h"
#include "cmAvrGccLanguageStandardOption.h"

namespace compiler {

bool CompilerOptionFactory::is_valid(const std::string& token)
{
  if (token[0] != '-') {
    return false;
  }

  const std::string prefixes = "WfODgwEnms";
  // Check if second character of token is part of the above characters list
  return (prefixes.find(token[1]) != std::string::npos);
}

std::vector<std::shared_ptr<CompilerOption>> CompilerOptionFactory::create(const std::string& token)
{
  std::vector<std::shared_ptr<CompilerOption>> out;
  if (!is_valid(token)) {
    return out;
  }

  switch (token[1]) {
    case 'O':
      if (OptimizationOption::can_create(token)) {
        out.push_back(std::make_shared<OptimizationOption>(token));
      }
      break;

    // Compile definitions
    case 'D':
      out.push_back(std::make_shared<DefinitionOption>(token));
      break;

    // Debug flags
    case 'g':
      if (DebugOption::can_create(token)) {
        out.push_back(std::make_shared<DebugOption>(token));
      }
      break;


    // Warning flags or linker flags (this is resolved with the help of the next character)
    case 'W':
      if (token[2] == 'l') {
        auto& split_tokens = LinkerOption::split_concatenated_options(token);
        for (auto& elem : split_tokens) {
          out.push_back(std::make_shared<LinkerOption>(elem));
        }
      }
      else
      {
        out.push_back(std::make_shared<WarningOption>(token));
      }
      break;

    // Machine-dependent options
    case 'm':
      out.push_back(std::make_shared<MachineOption>(token));
      break;

    // Language standard
    case 's':
      if (token.find("-std") != std::string::npos)
      {
        out.push_back(std::make_shared<LanguageStandardOption>(token));
      }
      break;

    // Generic, normal flags
    case 'f':
    case 'w':
    case 'n':
    default:
      out.push_back(std::make_shared<CompilerOption>(CompilerOption::Type::Generic, token));
      break;

  }

  return out;
}

void cmAvrGccCompiler::parse_flags(const std::string& flags)
{
  std::vector<std::string> tokens = cmutils::strings::split(flags);
  parse_flags(tokens);
}

std::vector<cmAvrGccCompiler::ShrdOption> cmAvrGccCompiler::get_options(const CompilerOption::Type type) const
{
  auto vec = options.storage.find(type);
  if (vec == options.storage.end())
  {
    return {};
  }
  return (*vec).second;
}

CompilerOption * cmAvrGccCompiler::get_option(const std::string& token) const
{
  return options.get_option(token).get();
}

std::vector<std::string> cmAvrGccCompiler::get_all_options(const CompilerOption::Type type) const
{
  OptionsVec options = get_options(type);
  std::vector<std::string> out;
  for (auto& opt : options)
  {
    out.push_back(opt->get_token());
  }
  return out;
}

void cmAvrGccCompiler::parse_flags(const std::vector<std::string>& tokens)
{
  for (const auto& token : tokens) {
    if (compiler::CompilerOptionFactory::is_valid(token)) {
      std::vector<ShrdOption> option_list = compiler::CompilerOptionFactory::create(token);
      for (auto& option : option_list) {
        this->options.accept_flag(option);
      }
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
  push_flag(flag, storage[flag->get_type()]);
}

std::vector<std::string> cmAvrGccCompiler::get_unsupported_options(const CompilerOption::Type type, const std::vector<std::string>& as7_options) const
{
  OptionsVec typed_options = get_options(type);
  std::vector<std::string> all_tokens;
  for (auto& option : typed_options) {
    all_tokens.push_back(option->get_token());
  }

  // Selectively copy only tokens which are not supported by atmel studio 7
  std::vector<std::string> out;
  for (auto& token : all_tokens) {
    if (std::find(as7_options.begin(), as7_options.end(), token) == as7_options.end()) {
      out.push_back(token);
    }
  }

  return out;
}

bool cmAvrGccCompiler::has_option(const std::string& option) const
{
  return options.contains(option);
}

void cmAvrGccCompiler::Options::clear()
{
  storage.clear();
}

void cmAvrGccCompiler::clear()
{
  options.clear();
}

cmAvrGccCompiler::Options::Options()
{
  // Registering options
  storage[CompilerOption::Type::Debug] = {};
  storage[CompilerOption::Type::Definition] = {};
  storage[CompilerOption::Type::Generic] = {};
  storage[CompilerOption::Type::Machine] = {};
  storage[CompilerOption::Type::Linker] = {};
  storage[CompilerOption::Type::Optimization] = {};
  storage[CompilerOption::Type::Warning] = {};
  storage[CompilerOption::Type::LanguageStandard] = {};
}

bool cmAvrGccCompiler::Options::contains(const std::string& token) const
{
  for (const auto& vec : storage )
  {
    if (contains(token, vec.second)){
      return true;
    }
  }
  return false;
}

bool cmAvrGccCompiler::Options::contains(const std::string& token, const OptionsVec& reference) const
{
  auto found_item = std::find_if(reference.begin(), reference.end(), [token](const ShrdOption& target)
  {
    return target->contains(token);
  });

  return (found_item != reference.end());
}


cmAvrGccCompiler::ShrdOption cmAvrGccCompiler::Options::get_option(const std::string& token) const
{
  ShrdOption out;
  for (const auto& vect : storage)
  {
    out = get_option(token, vect.second);
    if (out != nullptr)
    {
      break;
    }
  }
  return out;
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