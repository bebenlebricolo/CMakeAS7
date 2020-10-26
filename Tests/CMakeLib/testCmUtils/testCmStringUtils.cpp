#include <gtest/gtest.h>
#include <string>
#include "cmStringUtils.h"

namespace cmutilstests
{

TEST(StringUtils, test_string_strip)
{
    const std::string test_str1 = ".extensiontest";

    std::string out = cmutils::strings::strip(test_str1, '.');
    ASSERT_STREQ("extensiontest", out.c_str());

    out = cmutils::strings::strip(test_str1, 'e');
    ASSERT_STREQ(".xtnsiontst", out.c_str());
}

TEST(StringUtils, test_string_strip_multiple_chars)
{
    const std::string test_str1 = ".extensionteststring";

    std::string out = cmutils::strings::strip(test_str1, "es.nig");
    ASSERT_STREQ("xtotttr", out.c_str());
}

TEST(StringUtils, test_string_remove_duplicates)
{
    const std::string test_str1 = "abalebrkfioldalome";
    const std::string test_str2 = "I want some chocolate please !";

    std::string out = cmutils::strings::remove_duplicates(test_str1, true);
    ASSERT_STREQ("ablerkfiodm", out.c_str());

    out = cmutils::strings::remove_duplicates(test_str2, true);
    ASSERT_STREQ("I wantsomechlp!", out.c_str());
}

TEST(StringUtils, test_string_trim)
{
    const std::string test_str1 = "     Hello little world!     ";
    const std::string test_str2 = "~~~(((Hello little world!)))~~~";

    // Trimming string 1
    std::string out = cmutils::strings::trim(test_str1, ' ', cmutils::strings::TransformLocation::Start);
    ASSERT_STREQ("Hello little world!     ", out.c_str());

    out = cmutils::strings::trim(test_str1, ' ', cmutils::strings::TransformLocation::End);
    ASSERT_STREQ("     Hello little world!", out.c_str());

    out = cmutils::strings::trim(test_str1, ' ', cmutils::strings::TransformLocation::Both);
    ASSERT_STREQ("Hello little world!", out.c_str());

    // Trimming string 2
    out = cmutils::strings::trim(test_str2, '~', cmutils::strings::TransformLocation::Both);
    ASSERT_STREQ("(((Hello little world!)))", out.c_str());
    out = cmutils::strings::trim(out, '(', cmutils::strings::TransformLocation::Both);
    ASSERT_STREQ("Hello little world!)))", out.c_str());
    out = cmutils::strings::trim(out, ')', cmutils::strings::TransformLocation::Both);
    ASSERT_STREQ("Hello little world!", out.c_str());
}

TEST(StringUtils, test_string_replace_single_char)
{
  const std::string input_str = "This/is/a/path/to/nowhere";
  std::string out = cmutils::strings::replace(input_str, '/', '\\');

  ASSERT_EQ(out, "This\\is\\a\\path\\to\\nowhere");
  
  out = cmutils::strings::replace(out, '\\', '/');
  ASSERT_EQ(out, input_str);

  out = cmutils::strings::replace(out, 'a', 'A');
  ASSERT_EQ(out, "This/is/A/pAth/to/nowhere");
}

TEST(StringUtils, test_string_replace_full_strings)
{
  const std::string input_str1 = "Hello, my name is (place your name in here).";
  const std::string input_str2 = "Here is my age : %d";
  std::string out = cmutils::strings::replace(input_str1, "(place your name in here)", "Bob");

  ASSERT_EQ(out, "Hello, my name is Bob.");

  out = cmutils::strings::replace("A sentence with a lot of identical words, which is very long with a lot of identical patterns with a lot of identical characters", "a lot of identical", "some");
  ASSERT_EQ(out, "A sentence with some words, which is very long with some patterns with some characters");

  out = cmutils::strings::replace(input_str2, "%d", "123456");
  ASSERT_EQ(out, "Here is my age : 123456");

  out = cmutils::strings::replace("This is my %d age %d and %dIWantTo%dPut%dit%Deverywhere", "%d", "123456");
  ASSERT_EQ(out, "This is my 123456 age 123456 and 123456IWantTo123456Put123456it%Deverywhere");
}


} /* end of namespace cmutilstests*/

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}