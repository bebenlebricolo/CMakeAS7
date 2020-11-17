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

    const std::vector<ShrdCompilerOption>& get_options(const CompilerOption::Type type) const;

private:
    void accept_flag(const ShrdCompilerOption& flag);
    bool contains(const std::string& token, const std::vector<ShrdCompilerOption>& reference) const;
    bool is_unique(const std::string& token, const std::vector<ShrdCompilerOption>& reference) const;
    bool is_unique(const ShrdCompilerOption& flag, const std::vector<ShrdCompilerOption>& reference) const;

    std::vector<ShrdCompilerOption> optimizations;
    std::vector<ShrdCompilerOption> debug;
    std::vector<ShrdCompilerOption> warnings;
    std::vector<ShrdCompilerOption> linker;
    std::vector<ShrdCompilerOption> normal;

    // This is not the "standard" for cmake, but compiler can still interprete -D flags
    std::vector<ShrdCompilerOption> definitions;

};

}
