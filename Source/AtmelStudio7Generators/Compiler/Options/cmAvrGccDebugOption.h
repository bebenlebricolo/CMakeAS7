#pragma once

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include "cmAvrGccCompilerOption.h"

namespace compiler {

struct DebugOption : public CompilerOption
{
  enum class Level : uint8_t
  {
    None,   /**< no flags */
    g,      /**< -g       */
    g1,     /**< -g1      */
    g2,     /**< -g2      */
    g3,     /**< -g3      */
    ggdb,   /**< -ggdb    */
    gdwarf, /**< -gdwarf  */
  };


  /**
     * @brief determines whether the given token is part of the static map of available optimizations flags or not
     * @param[in]   _token : string representation of current flag being parsed
     * @return true : token exist in collection ; false : token is not part of the collection, thus it is not part of the available set of optimizations.
    */
  static bool can_create(const std::string& _token);

  /**
   * @brief returns the default value for debug flags
   * @return a pair of level and its corresponding representation
  */
  static std::pair<Level, AS7OptionRepresentation> get_default();

  DebugOption();
  DebugOption(const std::string& _token);

  bool operator==(const DebugOption& other) const;
  bool operator>=(const DebugOption& other) const;
  bool operator<=(const DebugOption& other) const;
  bool operator>(const DebugOption& other) const;
  bool operator<(const DebugOption& other) const;

  std::string generate(const bool atmel_studio_compat = true) override;


private:
  static std::unordered_map<Level, AS7OptionRepresentation> available_options;
  std::pair<Level, AS7OptionRepresentation*> resolve(const std::string& flag) const;
  Level optLevel = Level::None;
};

}
