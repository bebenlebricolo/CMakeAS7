#include <algorithm>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "cmAvrGccCompiler.h"
#include "cmAvrGccCompilerOption.h"
#include "cmAvrGccOptimizationOption.h"

namespace cmatmelstudiotests {

static bool check_flag_uniqueness(const std::vector<std::string>& target, const std::vector<std::shared_ptr<compiler::CompilerOption>>& reference)
{
  // Test that each input flag is unique inside selected flag vector from avr gcc compiler abstraction
  for (auto& flag : target) {
    auto nb_matches = std::count_if(reference.begin(), reference.end(), [flag](const std::shared_ptr<compiler::CompilerOption>& ref) {
      return flag == ref->get_token();
    });

    if (1 != nb_matches) {
      return false;
    }
  }
  return true;
}

TEST(AvrGccCompilerFlagsParsing, test_avr_gcc_representation)
{
  const std::string test_str1 = " -O3 -Og -Wall -Wextra -g2 -Wl,--gc-sections -gdwarf -fpack-struct -ggdb -DTEST_DEF=33 -g3 ";
  const std::vector<std::string> optimization_options = { "-O3", "-Og" };
  const std::vector<std::string> warning_options = { "-Wall", "-Wextra" };
  const std::vector<std::string> generic_options = { "-fpack-struct" };
  const std::vector<std::string> linker_options = { "-Wl,--gc-sections" };
  const std::vector<std::string> definition_options = { "-DTEST_DEF=33" };

  // https://gcc.gnu.org/onlinedocs/gcc/Debugging-Options.html
  const std::vector<std::string> debug_options = { "-g3", "-g2", "-gdwarf", "-ggdb" };

  compiler::cmAvrGccCompiler avrGcc;
  avrGcc.parse_flags(test_str1);

  ASSERT_TRUE(check_flag_uniqueness(optimization_options, avrGcc.get_optimization_flags()));
  ASSERT_TRUE(check_flag_uniqueness(warning_options, avrGcc.get_warning_flags()));
  ASSERT_TRUE(check_flag_uniqueness(generic_options, avrGcc.get_normal_flags()));
  ASSERT_TRUE(check_flag_uniqueness(linker_options, avrGcc.get_linker_flags()));
  ASSERT_TRUE(check_flag_uniqueness(definition_options, avrGcc.get_definitions()));
  ASSERT_TRUE(check_flag_uniqueness(debug_options, avrGcc.get_debug_flags()));
}

TEST(AvrGccCompilerFlagsParsing, test_optimization_flags)
{
  const std::vector<std::pair<std::string, compiler::OptimizationOption::Level>> valid_flags = {
    { "-O0", compiler::OptimizationOption::Level::O0 },
    { "-O", compiler::OptimizationOption::Level::O },
    { "-O1", compiler::OptimizationOption::Level::O1 },
    { "-O2", compiler::OptimizationOption::Level::O2 },
    { "-O3", compiler::OptimizationOption::Level::O3 },
    { "-Og", compiler::OptimizationOption::Level::Og },
    { "-Os", compiler::OptimizationOption::Level::Os },
    { "-Ofast", compiler::OptimizationOption::Level::Ofast },
  };
  const std::vector<std::string> invalid_flags = { "-ftoto", "-Wall", "-Wextra", "-Werror", "-pedantic", "-fpack-struct", "-ffunction-sections", "-DTEST_DEFINITION=33" };

  for (auto& f : valid_flags) {
    EXPECT_TRUE(compiler::OptimizationOption::can_create(f.first));
    compiler::OptimizationOption opt(f.first);
    EXPECT_EQ(opt.get_level(), f.second);
  }

  // Should not be able to create flags with invalid inputs
  for (auto& f : invalid_flags) {
    EXPECT_FALSE(compiler::OptimizationOption::can_create(f));
  }
}

TEST(AvrGccCompilerFlagsParsing, test_compiler_flags_factory_optimization_flags)
{
  const std::vector<std::string> flags = { "-Wall", "-DTEST_DEFINITION=33", "-Wextra", "-fpedantic", "-O2" };
  for (auto& f : flags) {
    EXPECT_TRUE(compiler::CompilerOptionFactory::is_valid(f));
  }

  std::shared_ptr<compiler::CompilerOption> built_flag = compiler::CompilerOptionFactory::create("-O2");
  ASSERT_NE(nullptr, built_flag);
  ASSERT_EQ(built_flag->GetType(), compiler::CompilerOption::Type::Optimization);
}

} /* end of namespace cmutilstests*/

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}