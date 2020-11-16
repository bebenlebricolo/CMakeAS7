#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "cmAvrGccCompilerOption.h"

namespace compiler
{

struct OptimizationFlag : public CompilerOption
{
    enum class Level : uint8_t
    {
        O0,     /**< No Optimizations whatsoever                            */
        O1,     /**< Optimization for code size and execution time          */
        O,      /**< Equivalent of O1                                       */
        O2,     /**< Optimize more for code size and execution time         */
        O3,     /**< Optimize more for code size and execution time         */
        Og,     /**< Optimize but keep enough information to help debugging */
        Os,     /**< Optimize for code size                                 */
        Ofast   /**< O3 with fast none accurate math calculations           */
    };

    /**
     * @brief determines whether the given token is part of the static map of available optimizations flags or not
     * @param[in]   _token : string representation of current flag being parsed
     * @return true : token exist in collection ; false : token is not part of the collection, thus it is not part of the available set of optimizations.
    */
    static bool can_create(const std::string& _token);

    OptimizationFlag() : CompilerOption(Type::Optimization){}
    OptimizationFlag(const std::string& _token);

    // Operators used to compare optimization levels, in order to resolve
    // the most important one when several are mistakenly passed to Cmake
    bool operator>(const OptimizationFlag& other) const;
    bool operator<(const OptimizationFlag& other) const;
    bool operator<=(const OptimizationFlag& other) const;
    bool operator>=(const OptimizationFlag& other) const;
    bool operator==(const OptimizationFlag& other) const;
    bool operator!=(const OptimizationFlag& other) const;

    Level get_level() const;
    std::string Generate(const bool atmel_studio_compat = true) override;

  private:
    struct FullDescription
    {
        FullDescription(const std::string& _flag, const std::string& _desc) : flag(_flag), atmel_studio_description(_desc) {}
        std::string flag;
        std::string atmel_studio_description;
    };

    static std::unordered_map<Level, FullDescription> available_opt;
    std::pair<Level, FullDescription*> resolve(const std::string& flag) const;
    Level optLevel = Level::O0;
};

}
