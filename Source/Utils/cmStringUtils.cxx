#include "cmStringUtils.h"

#include <algorithm>
#include <vector>
#include <deque>

namespace cmutils {

namespace strings {

std::string strip(const std::string& input_str, char c)
{
  std::string out;
  for (auto character : input_str) {
    if (character != c) {
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

std::vector<std::string> split(const std::string& input_str, const char delim)
{
  std::vector<std::string> vec;

  //Copy string content as we don't want to modify the input string content
  std::string cleaned_string = input_str;
  
  // remove contiguous duplicates which matches the delimiter character only
  auto it = std::unique(cleaned_string.begin(), cleaned_string.end(), [delim](const char a, const char b) {
      return (a == b && a == delim);
  });
  
  // Extract the useful part of cleaned string
  cleaned_string = std::string(cleaned_string.begin(), it);

  // Remove leading and trailing delimiter if any exist
  cleaned_string = trim(cleaned_string, delim, TransformLocation::Both);

  // Loop over the cleaned string to isolate substrings
  std::string substring;
  for (const char c : cleaned_string) {
    if (c != delim) {
      substring += c;
    } else {
      vec.push_back(substring);
      substring.clear();
    }
  }
 
  // Last substring is not pushed to vector when the last character is not a delimiter, 
  // So check if something remains in the substring object and add this to the output vector
  if (!substring.empty()) {
    vec.push_back(substring);
  }

  return vec;
}

std::string to_lowercase(const std::string& input_str)
{
  std::string out;
  out.reserve(input_str.size());
  for (char c : input_str) {
    out += std::tolower(c);
  }
  return out;
}

std::string to_uppercase(const std::string& input_str)
{
  std::string out;
  out.reserve(input_str.size());
  for (char c : input_str) {
    out += std::toupper(c);
  }
  return out;
}

std::string replace(const std::string& input_str, const char orig, const char replacement)
{
  std::string out = input_str;
  for (char& c : out) {
    // a bit of branchless programming, just for fun
    c = (c == orig) * replacement +
        (c != orig) * c;
  }
  return out;
}

std::string replace(const std::string& input_str, const std::string& orig, const std::string& replacement)
{
  std::string out;
  size_t local = 0;
  std::deque<unsigned int> locations;
  // Count occurrences first without touching to output string
  while (local != std::string::npos) {
    local = input_str.find(orig, local + 1);
    if (local != std::string::npos) {
      locations.push_back(local);
    }
  }
  
  // No more work to do if input sequence is not found
  if (locations.empty()) {
    return input_str;
  }

  out.reserve(input_str.size() + locations.size() * (replacement.size() - orig.size()));
  local = 0;
  while (local < input_str.size()) {
    if (local != locations.front()) {
      out += input_str[local];
      // next character
      ++local;
    } else {
      out += replacement;
      // Fast forward
      local += orig.size();
      // drop first element of the queue
      locations.pop_front();
    }
  }

  return out;
}

}

}