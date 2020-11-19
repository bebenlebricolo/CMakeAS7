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

    using ShrdOption = std::shared_ptr<CompilerOption>;
    using OptionsVec = std::vector<std::shared_ptr<CompilerOption>>;

    const OptionsVec& get_options(const CompilerOption::Type type) const;
    CompilerOption * get_option(const std::string& token) const;
    bool has_option(const std::string& option) const;
    void clear();

private:

    struct Options
    {
        bool contains(const std::string& token) const;
        bool contains(const std::string& token, const OptionsVec& reference) const;
        ShrdOption get_option(const std::string& token, const OptionsVec& vect) const;
        bool is_unique(const std::string& token, const OptionsVec& reference) const;
        bool is_unique(const ShrdOption& option, const OptionsVec& reference) const;
        void accept_flag(const ShrdOption& flag);
        void push_flag(const ShrdOption& flag, OptionsVec& vec);

        OptionsVec optimizations;
        OptionsVec debug;
        OptionsVec warnings;
        OptionsVec linker;
        OptionsVec normal;
        OptionsVec machine;
        // This is not the "standard" for cmake, but compiler can still interprete -D flags
        OptionsVec definitions;
    } options;
};

}
