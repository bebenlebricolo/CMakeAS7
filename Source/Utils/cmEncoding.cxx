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

#include "cmEncoding.h"

#include <algorithm>

namespace cmutils {

EncodingProperties::EncodingProperties()
  : encoding(Encoding::ASCII)
  , name("ASCII")
  , signature("")
  , max_frame_length(8U)
{
}

EncodingProperties::EncodingProperties(Encoding _encoding,
                                       const std::string& _name,
                                       const signature_t& _signature,
                                       const unsigned int _max_frame_length)
  : encoding(_encoding)
  , name(_name)
  , signature(_signature)
  , max_frame_length(_max_frame_length)
{
}

namespace EncodingHandler {
/**< Statically stores information about generic encodings */
static std::map<std::string, EncodingProperties> encoding_table = {
  { "ANSI",
    EncodingProperties(Encoding::ANSI, "ANSI",
                       EncodingProperties::signature_t(""), 8U) },
  { "ASCII",
    EncodingProperties(Encoding::ANSI, "ASCII",
                       EncodingProperties::signature_t(""), 7U) },
  { "UTF8",
    EncodingProperties(
      Encoding::ANSI, "UTF8",
      EncodingProperties::signature_t({ char(0xEF), char(0xBB), char(0xBF) }),
      32U) },

  { "UTF16",
    EncodingProperties(
      Encoding::ANSI, "ANSI",
      EncodingProperties::signature_t({ char(0xFE), char(0xFF) },
                                      { char(0xFF), char(0xFE) }),
      32U) },
};

static EncodingProperties default_properties()
{
  EncodingProperties prop;
  prop.encoding = Encoding::ASCII;
  prop.max_frame_length = 8U;
  prop.name = "ASCII";
  return prop;
}

EncodingProperties get_encoding_properties(Encoding encoding)
{
  auto found_item = std::find_if(
    encoding_table.begin(), encoding_table.end(),
    [encoding](const std::pair<std::string, EncodingProperties>& item) {
      return item.second.encoding == encoding;
    });

  // Create a default encoding object (ascii)
  if (encoding_table.end() == found_item) {
    return EncodingProperties();
  }

  return found_item->second;
}

EncodingProperties get_encoding_properties(const std::string& encoding_name)
{
  auto iterator = encoding_table.find(encoding_name);

  // Create a default encoding object (ascii)
  if (iterator == encoding_table.end()) {
    return EncodingProperties();
  }

  return iterator->second;
}

namespace compat
{
Encoding convert(const codecvt::Encoding encoding)
{
  switch (encoding) {
    case codecvt::Encoding::ANSI:
      return Encoding::ANSI;
    case codecvt::Encoding::UTF8:
      return Encoding::UTF8;
    default:
      return Encoding::ASCII;
  }
}

codecvt::Encoding convert(Encoding encoding)
{
  switch (encoding) {
    case Encoding::ANSI:
      return codecvt::Encoding::ANSI;
    case Encoding::UTF8:
      return codecvt::Encoding::UTF8;
    default:
      return codecvt::Encoding::None;
  }
}
} /* end of namespace compat */

}
}