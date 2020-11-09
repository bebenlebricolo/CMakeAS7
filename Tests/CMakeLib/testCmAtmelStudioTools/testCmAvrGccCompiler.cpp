#include <gtest/gtest.h>
#include <string>
#include <vector>
#include "cmAvrGccCompiler.h"
#include "cmAvrGccCompilerFlag.h"
#include "cmAvrGccOptimizationFlag.h"

namespace cmatmelstudiotests
{

//
//TEST(AvrGccCompilerFlagsParsing, test_avr_gcc_representation)
//{
//    GTEST_SKIP() << "Not implemented yet" << std::endl;
//    const std::string test_str1 = " -O3 -Og -Wall -Wextra -Wl,gc-sections -fpack-struct -DTEST_DEF=33 ";
//    compiler::cmAvrGccCompiler avrGcc;
//    avrGcc.parse_flags(test_str1);
//}

TEST(AvrGccCompilerFlagsParsing, test_optimization_flags)
{
    const std::vector<std::pair<std::string,compiler::OptimizationFlag::Level>> valid_flags =
    {
      {"-O0"    ,compiler::OptimizationFlag::Level::O0},
      {"-O"     ,compiler::OptimizationFlag::Level::O},
      {"-O1"    ,compiler::OptimizationFlag::Level::O1},
      {"-O2"    ,compiler::OptimizationFlag::Level::O2},
      {"-O3"    ,compiler::OptimizationFlag::Level::O3},
      {"-Og"    ,compiler::OptimizationFlag::Level::Og},
      {"-Os"    ,compiler::OptimizationFlag::Level::Os},
      {"-Ofast" ,compiler::OptimizationFlag::Level::Ofast},
    };
    const std::vector<std::string> invalid_flags = { "-ftoto", "-Wall", "-Wextra", "-Werror", "-pedantic", "-fpack-struct", "-ffunction-sections", "-DTEST_DEFINITION=33" };

    for (auto& f : valid_flags)
    {
      EXPECT_TRUE(compiler::OptimizationFlag::can_create(f.first));
      compiler::OptimizationFlag opt(f.first);
      EXPECT_EQ(opt.get_level(), f.second);
    }

    // Should not be able to create flags with invalid inputs
    for (auto& f : invalid_flags)
    {
      EXPECT_FALSE(compiler::OptimizationFlag::can_create(f));
    }

}

} /* end of namespace cmutilstests*/

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}