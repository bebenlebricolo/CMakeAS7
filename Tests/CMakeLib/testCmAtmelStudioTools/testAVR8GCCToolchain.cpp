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

namespace AtmelStudioToolsTests
{

class AVR8GCCTests : public ::testing::Test
{

};


// Tests that include paths are preserved when toolchain is enriched by
// the flag parser
TEST_F(AVR8GCCTests, toolchain_includes_persistence)
{
  AvrToolchain::AS7AvrGCC8 toolchain;
  const std::vector<std::string> include_dirs =
  {
    "include/temp dir 1",
    "include/temp dir 2",
    "include/temp dir 3",
    "include/temp dir 4",
  };

  // Add directories
  for (const auto& dir : include_dirs)
  {
    toolchain.avrgcc.directories.include_paths.push_back(dir);
  }


  compiler::cmAvrGccCompiler compiler_abstraction;
  //toolchain.convert_from()

}

}

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
