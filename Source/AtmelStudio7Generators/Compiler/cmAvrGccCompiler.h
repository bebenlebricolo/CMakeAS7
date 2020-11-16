#pragma once

#include <string>
#include <vector>
#include <memory>
#include "cmAvrGccCompilerOption.h"

namespace compiler
{

class cmAvrGccCompiler
{
public:
    void parse_flags(const std::vector<std::string>& tokens);
    void parse_flags(const std::string& flags);

    using ShrdCompilerOption = std::shared_ptr<CompilerOption>;

#ifdef UNIT_TESTING
    const std::vector<ShrdCompilerOption>& get_optimization_flags() const;
    const std::vector<ShrdCompilerOption>& get_debug_flags() const;
    const std::vector<ShrdCompilerOption>& get_warning_flags() const;
    const std::vector<ShrdCompilerOption>& get_linker_flags() const;
    const std::vector<ShrdCompilerOption>& get_normal_flags() const;
    const std::vector<ShrdCompilerOption>& get_definitions() const;
#endif

private:
    std::vector<ShrdCompilerOption> optimization_flags;
    std::vector<ShrdCompilerOption> debug_flags;
    std::vector<ShrdCompilerOption> warning_flags;
    std::vector<ShrdCompilerOption> linker_flags;
    std::vector<ShrdCompilerOption> normal_flags;

    // This is not the "standard" for cmake, but compiler can still interprete -D flags
    std::vector<ShrdCompilerOption> definitions;

    void accept_flag(const ShrdCompilerOption& flag);
};

}
