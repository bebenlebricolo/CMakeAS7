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

#pragma once

#include <map>
#include <string>

#include <cstring>
#include "cm_codecvt.hxx"

namespace cmutils {

#ifdef WIN32
#  define CMUTILS_API __declspec(dllexport)
#else
#  define CMUTILS_API
#endif

/**
 * @brief lists general purpose encodings
 */
enum class Encoding
{
  ASCII, /**< (Default) general purpose ASCII encoding (7 bit encoded
            characters, basic character set) */
  ANSI,  /**< general purpose ANSI encoding (8 bit encoded characters, basic
            character set)  */
  UTF8,  /**< UTF-8 encoding with extended character set    */
  UTF16, /**< UTF-16 encoding with extended character set   */
};

/**
 * @brief packs some properties about a given encoding
 */
struct EncodingProperties
{
  static const uint8_t signatureMaxSize = 10U;
  union signature_t
  {
    void clear()
    {
      memset(magic, 0, signatureMaxSize);
      memset(big_endian, 0, signatureMaxSize);
      memset(little_endian, 0, signatureMaxSize);
    }

    signature_t(const std::string& _magic)
    {
      clear();
      memcpy(magic, _magic.c_str(), signatureMaxSize);
    }

    signature_t(const std::string& _le, const std::string& _be)
    {
      clear();
      memcpy(little_endian, _le.c_str(), signatureMaxSize);
      memcpy(big_endian, _be.c_str(), signatureMaxSize);
    }

    signature_t(const signature_t& other)
    {
      memcpy(magic, other.magic, signatureMaxSize);
      memcpy(little_endian, other.little_endian, signatureMaxSize);
      memcpy(little_endian, other.big_endian, signatureMaxSize);
    }
    ~signature_t()
    {
      clear();
    }
    char magic[10]; /**< Regular magic chain  */
    struct
    {
      char little_endian[10]; /**< Little endian specific magic chain */
      char big_endian[10];    /**< Big endian specific magic chain    */
    };
  };

  EncodingProperties();
  EncodingProperties(Encoding _encoding, const std::string& _name,
                     const signature_t& _signature,
                     const unsigned int _max_frame_length);

  Encoding encoding;             /**< Enum key for this encoding             */
  std::string name;              /**< Encoding name              */
  unsigned int max_frame_length; /**< Represents the size in bits of a
                                    particular encoding. for instance, UTF-8
                                    uses a maximum of 32 bits    */
  signature_t signature;         /**< Represents the encoding's signature and/or
                                Byte Order Mask (BOM) */
};

/**
 * @brief Handles the encoding map which stores characteristics for each
 * encoding
 */
namespace EncodingHandler {

/**
 * @brief returns an EncodingProperties object using the Encoding enum as a
 * key
 * @param[in]   encoding        : Encoding key used to target one particular
 * encoding
 * @return EncodingProperties   : the resulting EncodingProperties structure,
 * of a default object if input key is not handled
 */
CMUTILS_API EncodingProperties get_encoding_properties(Encoding encoding);

/**
 * @brief returns an EncodingProperties object using the Encoding enum as a
 * key
 * @param[in]   encoding        : Encoding key used to target one particular
 * encoding
 * @return EncodingProperties   : the resulting EncodingProperties structure,
 * of a default object if input key is not handled
 */
CMUTILS_API EncodingProperties
get_encoding_properties(const std::string& encoding_name);

namespace compat {
/**
 * @brief converts from codecvt encoding to cmutils::Encoding enum class
 * @param encoding  : codecvt encoding formalism
 * @return Encoding cmutils encoding formalism
*/
CMUTILS_API Encoding convert(const codecvt::Encoding encoding);

/**
 * @brief converts from cmutils::Encoding to codecvt::Encoding enum
 * @param encoding  : cmutils::Encoding formalism
 * @return codecvt::Encoding formalism
*/
CMUTILS_API codecvt::Encoding convert(Encoding encoding);
} /* end of namespace compat */

};
}