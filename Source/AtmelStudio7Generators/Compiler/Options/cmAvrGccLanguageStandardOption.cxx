#include "cmAvrGccLanguageStandardOption.h"
#include "cmStringUtils.h"

namespace compiler
{

// see https://gcc.gnu.org/onlinedocs/gcc/C-Dialect-Options.html
std::vector<std::string> LanguageStandardOption::c_standards =
{
  "c89",
  "c90",
  "iso9899:1990",
  "gnu89",
  "gnu90",

  "c99",
  "c9x",
  "iso9899:1999",
  "iso9899:199x",
  "gnu99",
  "gnu9x",

  "c11",
  "c1x",
  "iso9899:2011",
  "iso9899:201x",
  "gnu11",
  "gnu1x",

  "c17",
  "c18",
  "iso9899:2017",
  "iso9899:2018",
  "gnu17",
  "gnu18",

  "c2x",
  "gnu2x",
};

std::vector<std::string> LanguageStandardOption::cpp_standards =
{
  "c++98",
  "c++03",
  "c++11",
  "c++0x",
  "c++14",
  "c++1y",
  "c++17",
  "c++1z",
  "c++20",
  "c++2a",

  "gnu++98",
  "gnu++03",
  "gnu++11",
  "gnu++0x",
  "gnu++14",
  "gnu++1y",
  "gnu++17",
  "gnu++1z",
  "gnu++20",
  "gnu++2a",
};


bool LanguageStandardOption::can_create(const std::string& _token)
{
  const auto vec = cmutils::strings::split(_token, '=');

  // Reject misformatted flag
  if (vec[0] != "-std")
  {
    return false;
  }

  return exists_in(c_standards, vec[1]) || exists_in(cpp_standards, vec[1]);
}

bool LanguageStandardOption::exists_in(const std::vector<std::string>& ref, std::string token)
{
  return std::find(ref.begin(), ref.end(), token) != ref.end();
}

LanguageStandardOption::LanguageStandardOption()
    : CompilerOption(Type::LanguageStandard)
{ }

LanguageStandardOption::LanguageStandardOption(const std::string& _token)
    : CompilerOption(Type::LanguageStandard)
{
  parse(_token);
}

void LanguageStandardOption::parse(const std::string& raw_token)
{
  std::vector<std::string> split = cmutils::strings::split(raw_token, '=');
  if (split[0] != "-std")
  {
    token = raw_token;
    value = "";
    lang = Language::Lang::Undefined;
  }
  else
  {
    if (exists_in(c_standards, split[1]))
    {
      lang = Language::Lang::C;
    }
    else
    {
      lang = Language::Lang::CXX;
    }

    token = raw_token;
    if (split.size() != 1)
    {
      value = split[1];
    }
  }
}


}
