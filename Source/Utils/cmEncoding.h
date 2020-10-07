#pragma once

#include <map>
#include <string>

namespace cmutils {

#ifdef WIN32
#  define CMUTILS_API __declspec(dllexport)
#else
    #define CMUTILS_API
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
  struct magic_signature
  {
    magic_signature(const std::string& _magic)
      : signature(_magic)
    {
    }
    magic_signature(const std::string& _little_endian,
                    const std::string& _big_endian)
      : signature(_little_endian, _big_endian)
    {
    }
    magic_signature(const magic_signature& other)
      : signature(other.signature)
    {
    }

    union signature_t
    {
      signature_t(const std::string& _magic)
        : magic(_magic)
      {
      }
      signature_t(const std::string& _le, const std::string& _be)
        : little_endian(_le)
        , big_endian(_be)
      {
      }
      signature_t(const signature_t& other)
      {
        magic = other.magic;
        little_endian = other.little_endian;
        big_endian = other.big_endian;
      }
      ~signature_t()
      {
        magic.clear();
        little_endian.clear();
        big_endian.clear();
      }
      std::string magic; /**< Regular magic chain  */
      struct
      {
        std::string little_endian; /**< Little endian specific magic chain */
        std::string big_endian;    /**< Big endian specific magic chain    */
      };
    } signature;
  };

  EncodingProperties();
  EncodingProperties(Encoding _encoding, const std::string& _name,
                     const magic_signature& _signature,
                     const unsigned int _max_frame_length);

  Encoding encoding;             /**< Enum key for this encoding             */
  std::string name;              /**< Encoding name              */
  unsigned int max_frame_length; /**< Represents the size in bits of a
                                    particular encoding. for instance, UTF-8
                                    uses a maximum of 32 bits    */
  magic_signature signature; /**< Represents the encoding's signature and/or
                                Byte Order Mask (BOM) */
};

/**
 * @brief Handles the encoding map which stores characteristics for each
 * encoding
 */
namespace EncodingHandler {

/**
 * @brief returns an EncodingProperties object using the Encoding enum as a key
 * @param[in]   encoding        : Encoding key used to target one particular
 * encoding
 * @return EncodingProperties   : the resulting EncodingProperties structure,
 * of a default object if input key is not handled
 */
CMUTILS_API EncodingProperties get_encoding_properties(Encoding encoding);

/**
 * @brief returns an EncodingProperties object using the Encoding enum as a key
 * @param[in]   encoding        : Encoding key used to target one particular
 * encoding
 * @return EncodingProperties   : the resulting EncodingProperties structure,
 * of a default object if input key is not handled
 */
CMUTILS_API EncodingProperties get_encoding_properties(
  const std::string& encoding_name);
};

}