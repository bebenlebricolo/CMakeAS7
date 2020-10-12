#include "cmStringUtils.h"

#include <algorithm>
#include <vector>

namespace cmutils {

namespace strings {

std::string strip(const std::string& input_str, char c)
{
  std::string out;
  for (auto character : input_str)
  {
    if (character != c){
      out += character;
    }
  }
  
  return out;
}

std::string strip(const std::string& input_str, const std::string& charset)
{
  std::string unique_charset = remove_duplicates(charset);
  std::string out = input_str;
  for (auto c : unique_charset) {
    out = strip(out, c);
  }
  return out;
}

std::string remove_duplicates(const std::string& input_str, bool keep_first)
{
  std::vector<char> unique_chars;
  std::string::const_iterator start;
  std::string::const_iterator end;

  if (keep_first) {
    start = input_str.begin();
    end = input_str.end();
  } else {
    start = input_str.end();
    end = input_str.begin();
  }

  // Takes all characters one by one and checks if they were already pushed in the unique characters vector
  std::string out;
  for (std::string::const_iterator it = start; it != end; it++) {
    bool has_character = (std::find(unique_chars.begin(), unique_chars.end(), *it) != unique_chars.end());
    if (!has_character) {
      out += *it;
      unique_chars.push_back(*it);
    }
  }

  return out;
}

std::string trim(const std::string& input_str, char c, TransformLocation tran)
{
  size_t first_char = input_str.find_first_not_of(c);
  size_t last_char = input_str.find_last_not_of(c);
  std::string out;

  // If we cannot find any other character, then return an empty string because 
  // trimming from any direction would given an empty string.
  if ((first_char == last_char) && (first_char == std::string::npos)) {
    out.clear();
    return out;
  }

  switch (tran) {
    case TransformLocation::Start:
      if (first_char != std::string::npos) {
        out = input_str.substr(first_char, input_str.size());
      }
      break;
    case TransformLocation::End:
      if (last_char != std::string::npos) {
        out = input_str.substr(0, last_char + 1);
      }
      break;
    default:
      out = input_str.substr(first_char, (last_char - first_char) + 1);
      break;
  }

  return out;
}

}

}