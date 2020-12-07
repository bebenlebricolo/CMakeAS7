#pragma once

#include <string>
#include <vector>

namespace AS7DeviceResolver
{

/**
 * @brief Lists all the cores supported in Atmel Studio 7
 */
enum class Core
{
    Unknown,        /**< No core specified / found, default case        */
    ATautomotive,   /**< Atmel automotive devices                       */
    ATmega,         /**< Atmel mega devices                             */
    AT90mega,       /**< Atmel 8 bit MCU, "mega" core with connectivity */
    ATtiny,         /**< Atmel tiny devices                             */
    ATxmega,        /**< Atmel xmega devices                            */
    SAM,            /**< Atmel SAM 32bits arm microcontrollers          */
    UC,             /**< Atmel 32 bits AVR devices                      */
};

/**
 * @brief applies the naming convention to the given input string using the detected core to perform the operation
 * @param core          :   detected chip's core
 * @param device_name   :   parsed device name, case does not matter but device name shall only contain
 *                          regular characters (no special char). E.g. : device_name = atmega328p ; __ATmega328P__ is not compliant.
 * @return device name formatted using the adequate naming convention
*/
std::string apply_naming_convention(Core core, const std::string& device_name);

/**
 * @brief resolves the Atmel Studio 7 device name using the -mmcu option passed to the compiler
 * @param   mmcu_option :   avr-gcc's "-mmcu=at...." option
 * @return converted device name based on the Atmel's naming conventions
*/
std::string resolve_from_mmcu(const std::string& mmcu_option);

/**
 * @brief resolves the Atmel Studio 7 device name using a collection of definitions passed to the compiler
 * @param   definitions : a collection of definitions passed to the compiler
 * @return converted device name based on the Atmel's naming conventions
*/
std::string resolve_from_defines(const std::vector<std::string>& definitions);

/**
 * @brief resolves the Atmel Studio 7 device name using a single definition passed to the compiler
 * @param   definitions : a collection of definitions passed to the compiler
 * @return converted device name based on the Atmel's naming conventions
*/
std::string resolve_from_defines(const std::string& definition);

}