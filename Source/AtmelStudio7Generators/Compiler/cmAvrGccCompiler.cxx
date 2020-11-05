#include "cmAvrGccCompiler.h"
#include "cmAvrGccOptimizationFlag.h"
#include "cmAvrGccWarningFlag.h"

namespace compiler
{

bool CompilerFlagFactory::is_valid(const std::string& token)
{
    if (token[0] != '-')
    {
        return false;
    }

    const std::string prefixes = "WfOD";
    // Check if second character of token is part of the above characters list
    return (prefixes.find(token[1]) != std::string::npos);
}

std::shared_ptr<CompilerFlag*> CompilerFlagFactory::create(const std::string& token)
{
    std::shared_ptr<CompilerFlag*> out;
    if (!is_valid(token))
    {
        return out;
    }

    switch(token[1])
    {
        case 'O':
            if (OptimizationFlag::can_create(token))
            {
                out = std::make_shared<CompilerFlag*>(new OptimizationFlag(token));
            }
            break;
        case 'D':
        case 'f':
        case 'W':
        default:
            break;
    }

    return out;
}


}