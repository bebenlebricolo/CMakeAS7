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

} /* end of namespace cmutilstests*/

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}