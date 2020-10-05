#include "cmEncoding.h"

#include <algorithm>

namespace cmutils {

EncodingProperties::EncodingProperties()
  : encoding(Encoding::ASCII)
  , name("ASCII")
  , signature(magic_signature(""))
  , max_frame_length(8U)
{
}

EncodingProperties::EncodingProperties(Encoding _encoding,
                                       const std::string& _name,
                                       const magic_signature& _signature,
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
                       EncodingProperties::magic_signature(""), 8U) },
  { "ASCII",
    EncodingProperties(Encoding::ANSI, "ASCII",
                       EncodingProperties::magic_signature(""), 7U) },
  { "UTF8",
    EncodingProperties(Encoding::ANSI, "UTF8",
                       EncodingProperties::magic_signature(
                         { char(0xEF), char(0xBB), char(0xBF) }),
                       32U) },

  { "UTF16",
    EncodingProperties(
      Encoding::ANSI, "ANSI",
      EncodingProperties::magic_signature({ char(0xFE), char(0xFF) },
                                          { char(0xFF), char(0xFE) }),
      32U) },
};

static EncodingProperties default_properties()
{
  EncodingProperties prop;
  prop.encoding = Encoding::ASCII;
  prop.max_frame_length = 8U;
  prop.name = "ASCII";
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

}
}