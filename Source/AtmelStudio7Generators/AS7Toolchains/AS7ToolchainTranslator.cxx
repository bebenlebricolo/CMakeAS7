#include "AS7ToolchainTranslator.h"
#include "pugixml.hpp"
#include "cmAvrGccCompiler.h"
#include "AvrGCC8Toolchain.h"

namespace AvrToolchain
{

void AS7ToolchainTranslator::parse(const std::vector<std::string>& flags, const std::string& lang)
{
    if (lang != "CXX" && lang != "C")
    {
        return;
    }

    // If Cxx is selected, then we need both C++ and C abstractions
    // However if C is selected and C++ has not been declared previously,
    // C++ abstraction is not needed.
    if (lang == "CXX")
    {
        targeted_language = lang;
    }
    else
    {
        if (targeted_language != "CXX")
        {
            targeted_language = lang;
        }
    }

    // Compiler abstraction is only instanciated when needed, previous
    // instance is cleared before parsing takes place.
    auto found_compiler = compilers.find(lang);
    if (found_compiler == compilers.end())
    {
        compiler::cmAvrGccCompiler cxxcompiler;
        cxxcompiler.parse_flags(flags);
        compilers[lang] = cxxcompiler;
    }
    else
    {
        found_compiler->second.clear();
        found_compiler->second.parse_flags(flags);
    }
}

std::string AS7ToolchainTranslator::get_targeted_language() const
{
  return targeted_language;
}

void AS7ToolchainTranslator::clear()
{
    toolchain.clear();
    compilers.clear();
}

void AS7ToolchainTranslator::generate_xml(pugi::xml_node& parent)
{
    sync_toolchain_languages();
    translate();
    toolchain.generate_xml(parent, targeted_language);
}

void AS7ToolchainTranslator::sync_toolchain_languages()
{
    if (get_compiler("C") == nullptr)
    {
        toolchain.avrgcc.copy_from(toolchain.avrgcccpp);
    }
}

void AS7ToolchainTranslator::translate()
{
    toolchain.convert_from(compilers["C"], "C");
    if (targeted_language == "CXX")
    {
        toolchain.convert_from(compilers["CXX"], "CXX");
    }
}

compiler::cmAvrGccCompiler* AS7ToolchainTranslator::get_compiler(const std::string& lang)
{
    auto found_compiler = compilers.find(lang);
    if (found_compiler != compilers.end())
    {
        return &(found_compiler->second);
    }
    return nullptr;
}

} /* end of namespace AvrToolchain */