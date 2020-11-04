#pragma once

#include <string>

namespace cmutils
{

namespace strings
{
    /**
     * @brief removes all occurrences of a character from the input string
     * @param input_str : input string
     * @param c         : character to be removed
     * @return std::string  : output string
     * @note output string is a copy of the original string -> not suitable for heavy stripping !
    */
    std::string strip(const std::string& input_str, char c);

    /**
     * @brief removes all occurrences of any characters of the previous list from the input string
     * @param input_str :   input string to be transformed
     * @param charset   :   set of characters to be removed from input string
     * @return transformed string
     * @note this algorithm uses the single shot version of strip described above.
     * As a consequence, its efficiency is not high, but still ok for small data sets
    */
    std::string strip(const std::string& input_str, const std::string& charset);

    /**
     * @brief Removes duplicates (unoptimized implementation, not suitable for heavy manipulations)
     *        This function removes all duplicates from a string, which does not mimic the behavior of
     *        std:unique (this one removes contiguous duplicates so that an input like { 1, 1, 2, 3, 4, 4, 5} will
     *        be transformed into { 1, 2, 3, 4, 5} by std::unique).
     *        Instead, this function will transform this { 1, 2, 3, 3, 4, 5, 3, 2, 1} into
     *                                                   { 1, 2, 3, 4, 5 }
     * @param input_str     :   input string to be transformed
     * @param keep_first    :   algorithm keeps the first unique character (reads the string from begining to end) if set to true
     *                          if set to false, the algorithm does the same but starting from the end of the string instead (keeps right-most character)
     * @return transformed string
     * @note this function uses a rather unefficient approach (but trivial) which collects unique characters from
     * input string and deletes (selectively copies) characters along the way into the output string
    */
    std::string remove_duplicates(const std::string& input_str, bool keep_first = true);

    /**
     * @brief Describes where a transformation should be executed on a string
    */
    enum class TransformLocation
    {
        Start,  /**< Transformation is operated at the start of the string (std::string::begin())   */
        End,    /**< Transformation is operated at the end of the string (std::string::end())       */
        Both    /**< Transformation is operated at both ends of the string                          */
    };

    /**
     * @brief Trims a string using a single character either from the Start, End of both ends of the string
     * @details This trim operation removes all occurrences of a single character from the input string,
     *          starting from the given location flag until reaching a character which is different.
     * @example
     *          input_str = "    i am a lonely string with whitespaces at both sides    "
     *          c = ' ' // Whitespace character
     *          tran = Start
     *             => Output string = "i am a lonely string with whitespaces at both sides    "
     *          tran = End
     *             => Output string = "    i am a lonely string with whitespaces at both sides"
     *          tran = Both
     *             => Output string = "i am a lonely string with whitespaces at both sides"
     * @param input_str :   input string to be trimmed
     * @param c         :   single character to be removed from input string
     * @param tran      :   transform location indicating where this operation has to be performed
     * @return trimmed string
    */
    std::string trim(const std::string& input_str, char c = ' ', TransformLocation tran = TransformLocation::Start);

    /**
     * @brief converts the input string to its lowercase version, char by char.
     * @param input_str :   input string to be transformed
     * @return transformed string
     */
    std::string to_lowercase(const std::string& input_str);

    /**
     * @brief converts the input string to its uppercase version, char by char.
     * @param input_str :   input string to be transformed
     * @return transformed string
     */
    std::string to_uppercase(const std::string& input_str);

    /**
     * @brief replaces an original character in the input string with the replacement character.
     * @param[in] orig          : original character to be replaced
     * @param[in] replacement   : replacement character replacing the original one
     * @return transformed string
     */
    std::string replace(const std::string& input_str, const char orig, const char replacement);

    /**
     * @brief replaces an original substring in the input string with the replacement string.
     * @param[in] orig          : original substring to be replaced
     * @param[in] replacement   : replacement substring replacing the original one
     * @return transformed string
     */
    std::string replace(const std::string& input_str, const std::string& orig, const std::string& replacement);

    }

}