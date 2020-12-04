#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
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

    OptionsVec get_options(const CompilerOption::Type type) const;
    CompilerOption * get_option(const std::string& token) const;

    /**
     * @brief returns a list of raw tokens contained within a collection of Options, minus the options that Atmel Studio supports.
     * This is used to compute the "Other flags" section to be filled within AtmelStudio7 project files
     *
     * @param   type        :   type of targeted options
     * @param   as7_options :   atmel studio 7 supported options list
     * @return  a vector of raw tokens
     */
    std::vector<std::string> get_unsupported_options(const CompilerOption::Type type, const std::vector<std::string>& as7_options) const;

    bool has_option(const std::string& option) const;
    void clear();

private:

    struct Options
    {
        Options();
        bool contains(const std::string& token) const;
        bool contains(const std::string& token, const OptionsVec& reference) const;
        ShrdOption get_option(const std::string& token, const OptionsVec& vect) const;
        ShrdOption get_option(const std::string& token) const;
        bool is_unique(const std::string& token, const OptionsVec& reference) const;
        bool is_unique(const ShrdOption& option, const OptionsVec& reference) const;
        void accept_flag(const ShrdOption& flag);
        void push_flag(const ShrdOption& flag, OptionsVec& vec);
        void clear();

        std::unordered_map<CompilerOption::Type, OptionsVec> storage;
    } options;
};

}
