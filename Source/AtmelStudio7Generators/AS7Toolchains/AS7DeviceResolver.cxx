#include "AS7DeviceResolver.h"

#include "cmStringUtils.h"
#include <regex>

namespace AS7DeviceResolver
{

std::string apply_naming_convention(Core core, const std::string& device_name)
{
    std::string out;
    std::string dev_up = cmutils::strings::to_uppercase(device_name);

    switch (core)
    {
        case Core::ATautomotive:
            {
                std::regex pattern("ATA([0-9]+)(.*)");
                std::smatch matches;
                std::regex_match(dev_up, matches, pattern);
                out = "ATA" + matches[1].str() + matches[2].str();
            }
            break;

        case Core::ATmega:
            {
                std::regex pattern("ATMEGA([0-9]+)(.*)");
                std::smatch matches;
                std::regex_match(dev_up, matches, pattern);
                out = "ATmega" + matches[1].str() + matches[2].str();
            }
            break;

        case Core::ATtiny:
            {
                std::regex pattern("ATTINY([0-9]+)(.*)");
                std::smatch matches;
                std::regex_match(dev_up, matches, pattern);

                // Special case handling for ATtiny416auto ...
                if (dev_up.find("AUTO") == std::string::npos)
                {
                    out = "ATtiny" + matches[1].str() + matches[2].str();
                }
                else
                {
                    out = "ATtiny" + matches[1].str() + "auto";
                }
            }
            break;

        case Core::ATxmega:
            {
                std::regex pattern("ATXMEGA([0-9]+)(.*)");
                std::smatch matches;
                std::regex_match(dev_up, matches, pattern);
                out = "ATxmega" + matches[1].str() + matches[2].str();
            }
            break;

        case Core::SAM:
        case Core::UC:
            out = cmutils::strings::to_uppercase(device_name);
            break;

        default:
            // unknown core, nothing to do
            break;
    }

    return out;
}

std::string resolve_from_mmcu(const std::string& mmcu_option)
{
  Core core = Core::Unknown;
  std::string option = cmutils::strings::to_lowercase(mmcu_option);

  // Take the right part of the '=' if it is still there (e.g. mmcu=atmega328pb)
  {
    auto vec = cmutils::strings::split(option, '=');
    if (vec.size() != 1) {
      option = vec[1];
    }
  }

  if (option.find("ata") != std::string::npos) {
    core = Core::ATautomotive;
  }

  if (option.find("atm") != std::string::npos) {
    core = Core::ATmega;
  }

  if (option.find("atx") != std::string::npos) {
    core = Core::ATxmega;
  }

  if (option.find("at90") != std::string::npos) {
    core = Core::AT90mega;
  }

  if (option.find("att") != std::string::npos) {
    core = Core::ATtiny;
  }

  if (option.find("at32") != std::string::npos) {
    core = Core::UC;
  }

  if (option.find("atsam") != std::string::npos) {
    core = Core::SAM;
  }

  return apply_naming_convention(core, option);
}

std::string resolve_from_defines(const std::vector<std::string>& definitions)
{
  for (auto& elem : definitions) {
    std::string device = resolve_from_defines(elem);
    if (!device.empty()) {
      return device;
    }
  }
  return "";
}

static std::string resolve_single_core(const std::string& input, const std::string& regex_pattern, const std::string& output_radical)
{
    std::string device;
    std::regex pattern;
    std::smatch matches;

    pattern.assign(regex_pattern);
    std::regex_match(input, matches, pattern);
    if (matches.size() > 1)
    {
        device = output_radical + matches[1].str();
    }

    return device;
}

std::string resolve_from_defines(const std::string& definition)
{
    // Reject definitions with values as MCU definitions does not use any
    if (definition.find("=") != std::string::npos)
    {
        return "";
    }

    std::string device;

    // Check for AVR cores (8 bit)
    device = resolve_single_core(definition, "__AVR_(.*)__", "");

    // Check for AVR32 cores (32 bit)
    if (device.empty())
    {
        device = resolve_single_core(definition, "__AVR32_UC(.*)__", "AT32UC");
    }

    // AT32 defines could as well be provided with __AT32UC...__ root
    // such as __AT32UC3A4256S__
    if (device.empty())
    {
        device = resolve_single_core(definition, "__AT32UC(.*)__", "AT32UC");
    }

    // SAM devices check
    if (device.empty())
    {
        device = resolve_single_core(definition, "__SAM(.*)__", "ATSAM");
    }

    return device;
}


}
