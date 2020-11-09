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
    using FlagContainer = std::shared_ptr<CompilerFlag>;
    std::vector<FlagContainer> optimization_flags;
    std::vector<FlagContainer> debug_flags;
    std::vector<FlagContainer> warning_flags;
    std::vector<FlagContainer> linker_flags;
    std::vector<FlagContainer> normal_flags;

    // This is not the "standard" for cmake, but compiler can still interprete -D flags
    std::vector<FlagContainer> definitions;

    void accept_flag(const FlagContainer& flag);
};

}
