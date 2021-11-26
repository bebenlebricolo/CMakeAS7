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

#include <gtest/gtest.h>
#include <filesystem>
#include "AS7DeviceResolver.h"

namespace cmAtmelStudioToolsTests
{

TEST(DeviceNamingConventionTest, test_Atmega_names)
{
  const std::vector<std::pair<std::string, std::string>> data =
  {
    {"atmega328p"       ,"ATmega328P"},
    {"atmega328pb"      ,"ATmega328PB"},
    {"atmega64a"        ,"ATmega64A"},
    {"atmega256rfr2"    ,"ATmega256RFR2"},
    {"atmega64hve2"    ,"ATmega64HVE2"}
  };

  for (auto& elem : data)
  {
    std::string convert = AS7DeviceResolver::apply_naming_convention(AS7DeviceResolver::Core::ATmega, elem.first);
    EXPECT_EQ(convert, elem.second);
  }
}

TEST(DeviceNamingConventionTest, test_Attiny_names)
{
  const std::vector<std::pair<std::string, std::string>> data =
  {
    {"attiny416auto"    ,"ATtiny416auto"},
    {"attiny85"         ,"ATtiny85"},
    {"attiny104"        ,"ATtiny104"},
    {"attiny3214"       ,"ATtiny3214"},
  };

  for (auto& elem : data)
  {
    std::string convert = AS7DeviceResolver::apply_naming_convention(AS7DeviceResolver::Core::ATtiny, elem.first);
    EXPECT_EQ(convert, elem.second);
  }
}

TEST(DeviceNamingConventionTest, test_Atautomotive_names)
{
  const std::vector<std::pair<std::string, std::string>> data =
  {
    {"ata6617c" ,"ATA6617C"},
    {"ata8510"  ,"ATA8510"},
    {"ata6617c" ,"ATA6617C"},
    {"ata5831"  ,"ATA5831"},
  };

  for (auto& elem : data)
  {
    std::string convert = AS7DeviceResolver::apply_naming_convention(AS7DeviceResolver::Core::ATautomotive, elem.first);
    EXPECT_EQ(convert, elem.second);
  }
}

// Note, not all ATA devices have their own -mmcu option (none that I could find in package.content though)
// So this test only checks naming convention applied to real cases of Atautomotive devices with their real -mmcu option
TEST(DeviceNamingConventionTest, test_Atxmega_names)
{
  const std::vector<std::pair<std::string, std::string>> data =
  {
    {"atxmega128a1u"  ,"ATxmega128A1U"},
    {"atxmega256a3bu" ,"ATxmega256A3BU"},
    {"atxmega32a4u"   ,"ATxmega32A4U"},
    {"atxmega64a1"    ,"ATxmega64A1"},
  };

  for (auto& elem : data)
  {
    std::string convert = AS7DeviceResolver::apply_naming_convention(AS7DeviceResolver::Core::ATxmega, elem.first);
    EXPECT_EQ(convert, elem.second);
  }
}

// Note, for AT32UC devices, there is no "-mmcu option", so the test below extrapolates Atmel's naming convention
TEST(DeviceNamingConventionTest, test_At32UC_names)
{
  const std::vector<std::pair<std::string, std::string>> data =
  {
    {"at32uc3a3256s"  ,"AT32UC3A3256S"},
    {"at32uc3a464"    ,"AT32UC3A464"},
    {"at32uc3a4256s"  ,"AT32UC3A4256S"},
    {"at32uc3a4128s"  ,"AT32UC3A4128S"},
  };

  for (auto& elem : data)
  {
    std::string convert = AS7DeviceResolver::apply_naming_convention(AS7DeviceResolver::Core::AT32UC, elem.first);
    EXPECT_EQ(convert, elem.second);
  }
}

TEST(DeviceNamingConventionTest, test_AT32UC_names_from_defines)
{
  const std::vector<std::pair<std::string, std::string>> data = {
    { "__AVR32_UC3A3256S__" , "AT32UC3A3256S" },
    { "__AT32UC3A3256S__"   , "AT32UC3A3256S" },
    { "__AVR32_UC3A464__"   , "AT32UC3A464" },
    { "__AT32UC3A464__"     , "AT32UC3A464" },
  };

  for (auto& elem : data) {
    std::string convert = AS7DeviceResolver::resolve_from_defines(elem.first);
    EXPECT_EQ(convert, elem.second);
  }
}

TEST(DeviceNamingConventionTest, test_SAM_names_from_defines)
{
  const std::vector<std::pair<std::string, std::string>> data =
  {
    {"__SAM3A4C__"  ,"ATSAM3A4C"},
    {"__SAM3N1A__"  ,"ATSAM3N1A"},
  };

  for (auto& elem : data)
  {
    std::string convert = AS7DeviceResolver::resolve_from_defines(elem.first);
    EXPECT_EQ(convert, elem.second);
  }
}

TEST(DeviceNamingConventionTest, test_mmcu_option_parsing)
{
  const std::vector<std::pair<std::string, std::string>> data = {
    { "-mmcu=atmega328p"  , "ATmega328P" },
    { "atmega328pb"       , "ATmega328PB" },
    { "-mmcu=attiny85"    , "ATtiny85" },
    { "atxmega256a3bu"    , "ATxmega256A3BU" },
    { "ata6617c"          , "ATA6617C" }
  };

  for (auto& elem : data) {
    std::string convert = AS7DeviceResolver::resolve_from_mmcu(elem.first);
    EXPECT_EQ(convert, elem.second);
  }
}

TEST(DeviceNamingConventionTest, test_definition_collection_resolving)
{
  std::vector<std::string> data = {
    "__FirstTest__",
    "VERBOSE=1",
    "NDEBUG",
    "__AVR_ATmega32YOP__=128", // should be discarded
    "__AVR_ATmega328PB__",
    "UNIT_TESTING=1"
  };

  std::string resolved = AS7DeviceResolver::resolve_from_defines(data);
  ASSERT_EQ(resolved, "ATmega328PB");
}

TEST(DeviceNamingConventionTest, test_DFP_resolution)
{
  std::vector<std::pair<std::string,std::string>> data = {
    { "ATSAM3A4C"     , "SAM3A_DFP" },
    { "ATSAM4N8C"     , "SAM4N_DFP" },
    { "ATSAMD10D14AM" , "SAMD10_DFP" },
    { "ATSAME51N19A"  , "SAME51_DFP" },
    { "ATSAMG51G18"   , "SAMG_DFP" },

    { "ATmega328PB"   , "ATmega_DFP" },
    { "ATtiny85"      , "ATtiny_DFP" },
    { "ATa6613c"      , "ATautomotive_DFP" },
    { "ATtiny416auto" , "ATautomotive_DFP" }, //  Special case

    { "ATxmega32A4U"  , "XMEGAA_DFP" },
    { "ATxmega64B1"   , "XMEGAB_DFP" },
    { "ATxmega64D3"   , "XMEGAD_DFP" },
    { "ATxmega16E5"   , "XMEGAE_DFP" },

    { "AT32UC3A4256S" , "UC3A_DFP" },
    { "AT32UC3B0512"  , "UC3B_DFP" },
    { "ATUC128D3"     , "UC3D_DFP" },
    { "ATUC128L3"     , "UC3L_DFP" },

  };

  for (auto& elem : data) {
    std::string resolved = AS7DeviceResolver::resolve_device_dfp_name(elem.first);
    EXPECT_EQ(resolved, elem.second);
  }
}


TEST(DeviceNamingConventionTest, test_version_finder)
{
  std::vector<std::string> data = {
    "1.1.1",
    "1.2.1",
    "1.1.3",
    "4.1.1",
    "5.0.0",
  };

  auto temp_folder = std::filesystem::temp_directory_path();
  auto base_path = temp_folder / "AS7DeviceResolverTests";

  if (!std::filesystem::exists(base_path)) {
    ASSERT_TRUE(std::filesystem::create_directory(base_path));
  }

  // Create folders
  for (auto& dir : data) {
    if (!std::filesystem::exists(base_path / dir)) {
      ASSERT_TRUE(std::filesystem::create_directory(base_path / dir));
    }
  }

  ASSERT_EQ(AS7DeviceResolver::get_max_packs_version(base_path.string()), "5.0.0");
}

}

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
