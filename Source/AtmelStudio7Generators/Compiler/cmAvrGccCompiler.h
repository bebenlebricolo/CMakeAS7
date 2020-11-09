#pragma once

#include <string>
#include <vector>
#include <memory>
#include "cmAvrGccCompilerFlag.h"

namespace compiler
{

class cmAvrGccCompiler
{
public:
    void parse_flags(const std::vector<std::string>& tokens);
    void parse_flags(const std::string& flags);
private:
    std::vector<std::shared_ptr<CompilerFlag*>> optimization_flags;
    std::vector<std::shared_ptr<CompilerFlag*>> debug_flags;
    std::vector<std::shared_ptr<CompilerFlag*>> warning_flags;
    std::vector<std::shared_ptr<CompilerFlag*>> linker_flags;
    std::vector<std::shared_ptr<CompilerFlag*>> normal_flags;

    // This is not the "standard" for cmake, but compiler can still interprete -D flags
    std::vector<std::shared_ptr<CompilerFlag*>> definitions;
};

}
