/*
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3.0 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <algorithm>
#include <string>
#include <vector>
#include <filesystem>

#include <gtest/gtest.h>

#include "cmAvrGccCompiler.h"
#include "cmAvrGccCompilerOption.h"
#include "cmAvrGccOptimizationOption.h"
#include "cmAvrGccMachineOption.h"
#include "cmAvrGccDebugOption.h"
#include "cmAvrGccLinkerOption.h"
#include "AvrGCC8Toolchain.h"
#include "AS7ToolchainTranslator.h"
#include "cmStringUtils.h"
#include "pugixml.hpp"

namespace cmAtmelStudioToolsTests {

class FlagParsingFixture : public ::testing::Test
{
public:
  void SetUp() override
  {

    std::vector<std::string> cflags = { "-Wall", "-DTEST_DEFINITION=33", "-Wextra",
                                    "-fpedantic", "-O2", "-O3", "-O0", "-g1", "-g2", "-g3", "-ffunction-sections",
                                    "-fpedantic-errors", "-mcall-prologues", "-mno-interrupts", "-funsigned-char",
                                    "-nostdinc", "-fpack-struct", "-mshort-calls", "-mmcu=atmega328p", "-Wl,--gc-sections,-relax" };

    std::vector<std::string> cppflags = { "-Wall", "-DTEST_C++=148", "-Wextra",
                                          "-fpedantic", "-O2", "-g1", "-g3", "-ffunction-sections",
                                          "-fpedantic-errors", "-mcall-prologues", "-mno-interrupts", "-funsigned-char",
                                          "-nostdinc", "-fpack-struct", "-mshort-calls" , "-flto", "-fno-exceptions", "-fno-rtti", "-Wa,-g"};

    toolchain_translator.parse(cflags, "C");
    toolchain_translator.parse(cppflags, "CXX");
  }

  void TearDown() override
  {
    toolchain_translator.clear();
  }

protected:
  AvrToolchain::AS7ToolchainTranslator toolchain_translator;
};

static bool check_flag_uniqueness(const std::vector<std::string>& target, const std::vector<std::shared_ptr<compiler::CompilerOption>>& reference)
{
  // Test that each input flag is unique inside selected flag vector from avr gcc compiler abstraction
  for (auto& flag : target) {
    auto nb_matches = std::count_if(reference.begin(), reference.end(), [flag](const std::shared_ptr<compiler::CompilerOption>& ref) {
      return ref->contains(flag);
    });

    if (1 != nb_matches) {
      return false;
    }
  }
  return true;
}

static void compare_misc(const std::string& ref, const std::string& target)
{
  auto ref_vec = cmutils::strings::split(ref);
  auto tgt_vec = cmutils::strings::split(target);

  for (const auto& tgt : tgt_vec)
  {
    auto item = std::find(ref_vec.begin(), ref_vec.end(), tgt);
    EXPECT_NE(item, ref_vec.end());
  }
}

TEST(AvrGccCompilerFlagsParsing, test_avr_gcc_representation)
{
  const std::string test_str1 = " -O3 -Og -Wall -Wall -Wextra -Wextra -g2 -g2 -Wl,--gc-sections -Wl,--gc-sections -gdwarf -gdwarf -fpack-struct -fpack-struct -ggdb -DTEST_DEF=33 -DTEST_DEF=33 -DTEST_DEF=33 -g3 ";
  const std::vector<std::string> optimization_options = { "-O3", "-Og" };
  const std::vector<std::string> warning_options = { "-Wall", "-Wextra" };
  const std::vector<std::string> generic_options = { "-fpack-struct" };
  const std::vector<std::string> linker_options = { "-Wl,--gc-sections" };
  const std::vector<std::string> definition_options = { "-DTEST_DEF=33" };

  // https://gcc.gnu.org/onlinedocs/gcc/Debugging-Options.html
  const std::vector<std::string> debug_options = { "-g3", "-g2", "-gdwarf", "-ggdb" };

  compiler::cmAvrGccCompiler avrGcc;
  avrGcc.parse_flags(test_str1);

  ASSERT_TRUE(check_flag_uniqueness(optimization_options, avrGcc.get_options(compiler::CompilerOption::Type::Optimization)));
  ASSERT_TRUE(check_flag_uniqueness(warning_options, avrGcc.get_options(compiler::CompilerOption::Type::Warning)));
  ASSERT_TRUE(check_flag_uniqueness(generic_options, avrGcc.get_options(compiler::CompilerOption::Type::Generic)));
  ASSERT_TRUE(check_flag_uniqueness(linker_options, avrGcc.get_options(compiler::CompilerOption::Type::Linker)));
  ASSERT_TRUE(check_flag_uniqueness(definition_options, avrGcc.get_options(compiler::CompilerOption::Type::Definition)));
  ASSERT_TRUE(check_flag_uniqueness(debug_options, avrGcc.get_options(compiler::CompilerOption::Type::Debug)));
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

TEST(AvrGccCompilerFlagsParsing, test_linker_concatenated_options)
{
  const std::string concatenated_options = { "-Wl,--gc-sections,--relax"};
  compiler::cmAvrGccCompiler compiler_abstraction;
  compiler_abstraction.parse_flags(concatenated_options);

  ASSERT_TRUE(compiler_abstraction.has_option("Wl,--gc-sections"));
  ASSERT_TRUE(compiler_abstraction.has_option("--gc-sections"));
  ASSERT_TRUE(compiler_abstraction.has_option("Wl,--relax"));
  ASSERT_TRUE(compiler_abstraction.has_option("--relax"));

  const auto& linker_options = compiler_abstraction.get_options(compiler::CompilerOption::Type::Linker);
  ASSERT_EQ(linker_options.size(), 2);

}

TEST(AvrGccCompilerFlagsParsing, test_machine_options)
{

  const std::vector<std::string> valid_flags = { "-mmcu", "-mcall-prologues", "-mint8", "-mno-interrupts", "-mtiny-stack"};
  const std::vector<std::string> invalid_flags = { "-ftoto", "-Wall", "-Wextra", "-Werror", "-pedantic", "-fpack-struct", "-ffunction-sections", "-DTEST_DEFINITION=33" };

  for (auto& f : valid_flags) {
    EXPECT_TRUE(compiler::MachineOption::can_create(f));
  }

  compiler::MachineOption opt("-mmcu=atmega328p");
  ASSERT_EQ(opt.value, "atmega328p");

  // Should not be able to create flags with invalid inputs
  for (auto& f : invalid_flags) {
    EXPECT_FALSE(compiler::OptimizationOption::can_create(f));
  }
}

TEST(AvrGccCompilerFlagsParsing, test_optimization_default_flags)
{
  auto default_opt = compiler::OptimizationOption::get_default();
  ASSERT_EQ(default_opt.first, compiler::OptimizationOption::Level::Os);
  ASSERT_EQ(default_opt.second.atmel_studio_description, "Optimize for size (-Os)");
  ASSERT_EQ(default_opt.second.option, "-Os");
}

TEST(AvrGccCompilerFlagsParsing, test_debug_default_flags)
{
  auto default_opt = compiler::DebugOption::get_default();
  ASSERT_EQ(default_opt.first, compiler::DebugOption::Level::None);
  ASSERT_EQ(default_opt.second.atmel_studio_description, "None");
  ASSERT_EQ(default_opt.second.option, "");
}

TEST(AvrGccCompilerFlagsParsing, test_compiler_flags_factory_optimization_flags)
{
  const std::vector<std::string> flags = { "-Wall", "-DTEST_DEFINITION=33", "-Wextra", "-fpedantic", "-O2" };
  for (auto& f : flags) {
    EXPECT_TRUE(compiler::CompilerOptionFactory::is_valid(f));
  }

  std::vector<std::shared_ptr<compiler::CompilerOption>> built_flag = compiler::CompilerOptionFactory::create("-O2");
  ASSERT_EQ(built_flag.size(), 1);
  EXPECT_NE(nullptr, built_flag[0]);
  ASSERT_EQ(built_flag[0]->get_type(), compiler::CompilerOption::Type::Optimization);
}

TEST(AvrGcc8Representation, convert_from_compiler_abstraction_all_ok)
{
  const std::vector<std::string> flags = { "-Wall", "-DTEST_DEFINITION=33", "-Wextra", "-Werror", "-pedantic", "-pedantic-errors",
                                           "-fpedantic", "-O2", "-O3", "-O0", "-g1", "-g2", "-g3", "-ffunction-sections",
                                           "-fpedantic-errors", "-mcall-prologues", "-mno-interrupts", "-funsigned-char",
                                           "-nostdinc", "-fpack-struct", "-mshort-calls", "-std=c11" };
  for (auto& f : flags)
  {
    bool valid = compiler::CompilerOptionFactory::is_valid(f);
    EXPECT_TRUE(valid);
    if (!valid)
    {
      std::cout << "invalid flag is : " << f.c_str() << "\n";
    }
  }

  compiler::cmAvrGccCompiler compiler_abstraction;
  compiler_abstraction.parse_flags(flags);

  AvrToolchain::AS7AvrGCC8 toolchain;
  pugi::xml_node node;
  toolchain.convert_from(compiler_abstraction);

  // Checking the general abstraction of avrgcc
  ASSERT_TRUE(toolchain.avrgcc.general.subroutine_function_prologue);
  ASSERT_TRUE(toolchain.avrgcc.general.change_stack_pointer_without_disabling_interrupt);
  ASSERT_TRUE(toolchain.avrgcc.general.change_default_chartype_unsigned);
  ASSERT_FALSE(toolchain.avrgcc.general.change_default_bitfield_unsigned);

  // Checking preprocessor abstraction of avrgcc
  ASSERT_TRUE(toolchain.avrgcc.preprocessor.do_not_search_system_directories);
  ASSERT_FALSE(toolchain.avrgcc.preprocessor.preprocess_only);

  // Checking symbols abstraction of avrgcc
  ASSERT_FALSE(toolchain.avrgcc.symbols.def_symbols.empty());
  //ASSERT_EQ(toolchain.avrgcc.symbols.def_symbols, "TEST_DEFINITION=33");

  // Checking directories abstraction of avrgcc
  ASSERT_TRUE(toolchain.avrgcc.directories.include_paths.empty());

  // Checking optimizations abstraction of avrgcc
  ASSERT_EQ(toolchain.avrgcc.optimizations.level, "None (-O0)");
  ASSERT_TRUE(toolchain.avrgcc.optimizations.other_flags.empty());
  ASSERT_TRUE(toolchain.avrgcc.optimizations.prepare_function_for_garbage_collection);
  ASSERT_FALSE(toolchain.avrgcc.optimizations.prepare_data_for_garbage_collection);
  ASSERT_TRUE(toolchain.avrgcc.optimizations.pack_structure_members);
  ASSERT_FALSE(toolchain.avrgcc.optimizations.allocate_bytes_needed_for_enum);
  ASSERT_TRUE(toolchain.avrgcc.optimizations.use_short_calls);
  ASSERT_FALSE(toolchain.avrgcc.optimizations.debug_level.empty());
  ASSERT_EQ(toolchain.avrgcc.optimizations.debug_level, "Maximum (-g3)");
  ASSERT_TRUE(toolchain.avrgcc.optimizations.other_debugging_flags.empty());

  // Checking warning abstraction of avrgcc
  ASSERT_TRUE(toolchain.avrgcc.warnings.all_warnings);
  ASSERT_TRUE(toolchain.avrgcc.warnings.extra_warnings);
  ASSERT_FALSE(toolchain.avrgcc.warnings.undefined);
  ASSERT_TRUE(toolchain.avrgcc.warnings.warnings_as_error);
  ASSERT_FALSE(toolchain.avrgcc.warnings.check_syntax_only);
  ASSERT_TRUE(toolchain.avrgcc.warnings.pedantic);
  ASSERT_TRUE(toolchain.avrgcc.warnings.pedantic_warnings_as_errors);
  ASSERT_FALSE(toolchain.avrgcc.warnings.inhibit_all_warnings);
  ASSERT_TRUE(toolchain.avrgcc.warnings.other_warnings.empty());

  // Checking miscellaneous abstraction of avrgcc
  ASSERT_FALSE(toolchain.avrgcc.miscellaneous.other_flags.empty());
  ASSERT_FALSE(toolchain.avrgcc.miscellaneous.verbose);
  ASSERT_FALSE(toolchain.avrgcc.miscellaneous.support_ansi_programs);
  ASSERT_FALSE(toolchain.avrgcc.miscellaneous.do_not_delete_temporary_files);
  compare_misc(toolchain.avrgcc.miscellaneous.other_flags, "-std=c11 -fpedantic -fpedantic-errors");

}

TEST(AvrGcc8Representation, convert_from_compiler_abstraction_misc_flags_redirection)
{
  const std::vector<std::string> flags = { "-Wall", "-DTEST_DEFINITION=33", "-Wextra",
    "-fpedantic", "-O2", "-O3", "-O0", "-g1", "-g2", "-g3", "-ffunction-sections",
    "-fpedantic-errors", "-mcall-prologues", "-mno-interrupts", "-funsigned-char",
    "-nostdinc", "-fpack-struct", "-mshort-calls", "-std=c11",
    "-Wno-unused-parameter", "-Wno-error=unused-function", "-Wno-error=unused-variable"};

  const std::string expected_misc_flags = "-Wno-unused-parameter -Wno-error=unused-function -Wno-error=unused-variable -fpedantic -fpedantic-errors -std=c11";

  for (auto& f : flags)
  {
    EXPECT_TRUE(compiler::CompilerOptionFactory::is_valid(f));
  }


  compiler::cmAvrGccCompiler compiler_abstraction;
  compiler_abstraction.parse_flags(flags);

  AvrToolchain::AS7AvrGCC8 toolchain;
  pugi::xml_node node;
  toolchain.convert_from(compiler_abstraction);
  ASSERT_TRUE(toolchain.avrgcc.general.change_default_chartype_unsigned);
  ASSERT_TRUE(toolchain.avrgcc.general.change_stack_pointer_without_disabling_interrupt);
  ASSERT_TRUE(toolchain.avrgcc.general.subroutine_function_prologue);
  ASSERT_EQ(toolchain.avrgcc.optimizations.level, "None (-O0)");
  ASSERT_TRUE(toolchain.avrgcc.optimizations.pack_structure_members);
  ASSERT_FALSE(toolchain.avrgcc.optimizations.prepare_data_for_garbage_collection);
  ASSERT_TRUE(toolchain.avrgcc.optimizations.prepare_function_for_garbage_collection);
  compare_misc(toolchain.avrgcc.miscellaneous.other_flags, expected_misc_flags);
}

TEST_F(FlagParsingFixture, test_generate_xml)
{
  pugi::xml_document doc;
  // Prepend delcaration node will look like this : <?xml version="1.0" encoding="utf-8"?>
  pugi::xml_node project_node = doc.append_child(pugi::node_element);
  toolchain_translator.generate_xml(project_node);
  doc.save_file((std::filesystem::temp_directory_path() / "CMakeLibTests" / "AtmelStudio7Tools" / "testfile.xml").c_str());

}


}

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
