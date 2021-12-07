/*
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3.0 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

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
        compilers[lang] = std::unique_ptr<compiler::AbstractCompilerModel>(new compiler::cmAvrGccCompiler());
        compilers[lang]->parse_flags(flags);
    }
    else
    {
        found_compiler->second->clear();
        found_compiler->second->parse_flags(flags);
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

// NOTE : better switch to enum versions of the languages : plain strings are not a good solution.
void AS7ToolchainTranslator::translate()
{
    toolchain.convert_from(*compilers["C"], "C");
    if (targeted_language == "CXX")
    {
        toolchain.convert_from(*compilers["CXX"], "CXX");
    }
}

compiler::AbstractCompilerModel* AS7ToolchainTranslator::get_compiler(const std::string& lang)
{
    auto found_compiler = compilers.find(lang);
    if (found_compiler != compilers.end())
    {
        return (found_compiler->second.get());
    }
    return nullptr;
}

} /* end of namespace AvrToolchain */