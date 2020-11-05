#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_set>

#include "cmAvrGccCompilerFlag.h"

namespace compiler
{

struct WarningFlag : public CompilerFlag
{
    /**
     * @brief determines whether the given token is part of the static map of available optimizations flags or not
     * @param[in]   _token : string representation of current flag being parsed
     * @return true : token exist in collection ; false : token is not part of the collection, thus it is not part of the available set of optimizations.
    */
    static bool can_create(const std::string& _token);

    WarningFlag() : CompilerFlag(Type::Warning){}
    WarningFlag(const std::string& _token);

private:
    static std::unordered_set<std::string> available_flags;
};

}
