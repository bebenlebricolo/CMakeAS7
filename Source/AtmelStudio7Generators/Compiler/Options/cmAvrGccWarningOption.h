#pragma once

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include "cmAvrGccCompilerFlag.h"

namespace compiler {

struct WarningOption : public CompilerFlag
{
  /**
     * @brief determines whether the given token is part of the static map of available optimizations flags or not
     * @param[in]   _token : string representation of current flag being parsed
     * @return true : token exist in collection ; false : token is not part of the collection, thus it is not part of the available set of optimizations.
    */
  static bool can_create(const std::string& _token) { return true; }

  WarningOption()
    : CompilerFlag(Type::Warning)
  {
  }
  WarningOption(const std::string& _token)
    : CompilerFlag(Type::Warning, _token)
  {
  }

private:
  static std::unordered_set<std::string> available_flags;
};

}
