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

#include "AS7DeviceResolver.h"

#include <algorithm>
#include <filesystem>
#include <numeric>
#include <regex>
#include <vector>
#include <cmath>

#include "cmStringUtils.h"

namespace AS7DeviceResolver {

std::string apply_naming_convention(Core core, const std::string& device_name)
{
  std::string out;
  std::string dev_up = cmutils::strings::to_uppercase(device_name);

  switch (core) {
    case Core::ATautomotive: {
      std::regex pattern("ATA([0-9]+)(.*)");
      std::smatch matches;
      std::regex_match(dev_up, matches, pattern);
      out = "ATA" + matches[1].str() + matches[2].str();
    } break;

    case Core::ATmega: {
      std::regex pattern("ATMEGA([0-9]+)(.*)");
      std::smatch matches;
      std::regex_match(dev_up, matches, pattern);
      out = "ATmega" + matches[1].str() + matches[2].str();
    } break;

    case Core::AT90mega: {
      out = dev_up;
    } break;

    case Core::ATtiny: {
      std::regex pattern("ATTINY([0-9]+)(.*)");
      std::smatch matches;
      std::regex_match(dev_up, matches, pattern);

      // Special case handling for ATtiny416auto ...
      if (dev_up.find("AUTO") == std::string::npos) {
        out = "ATtiny" + matches[1].str() + matches[2].str();
      } else {
        out = "ATtiny" + matches[1].str() + "auto";
      }
    } break;

    case Core::ATxmega: {
      std::regex pattern("ATXMEGA([0-9]+)(.*)");
      std::smatch matches;
      std::regex_match(dev_up, matches, pattern);
      out = "ATxmega" + matches[1].str() + matches[2].str();
    } break;

    case Core::ATSAM:
    case Core::AT32UC:
      out = cmutils::strings::to_uppercase(device_name);
      break;

    case Core::Unknown:
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

  core = resolve_core_from_name(option);

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
  if (matches.size() > 1) {
    device = output_radical + matches[1].str();
  }

  return device;
}

std::string resolve_from_defines(const std::string& definition)
{
  // Reject definitions with values as MCU definitions does not use any
  if (definition.find("=") != std::string::npos) {
    return "";
  }

  std::string device;

  // Check for AVR cores (8 bit)
  device = resolve_single_core(definition, "__AVR_(.*)__", "");

  // Check for AVR32 cores (32 bit)
  if (device.empty()) {
    device = resolve_single_core(definition, "__AVR32_UC(.*)__", "AT32UC");
  }

  // AT32 defines could as well be provided with __AT32UC...__ root
  // such as __AT32UC3A4256S__
  if (device.empty()) {
    device = resolve_single_core(definition, "__AT32UC(.*)__", "AT32UC");
  }

  // SAM devices check
  if (device.empty()) {
    device = resolve_single_core(definition, "__SAM(.*)__", "ATSAM");
  }

  return device;
}

Core resolve_core_from_name(const std::string& device_name)
{
  Core core = Core::Unknown;

  std::string option = cmutils::strings::to_lowercase(device_name);

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

  // There are 2 available naming conventions for those AVR32 cores ... !
  if ((option.find("at32") != std::string::npos) || (option.find("atuc") != std::string::npos)) {
    core = Core::AT32UC;
  }

  if (option.find("atsam") != std::string::npos) {
    core = Core::ATSAM;
  }

  return core;
}

static std::string resolve_sam_dfps(const std::string& device_name)
{
  std::string out;
  std::regex pattern("ATSAM([0-9][A-Z]).*");
  std::smatch matches;
  std::regex_match(device_name, matches, pattern);

  if (!matches.empty()) {
    out = "SAM" + matches[1].str() + "_DFP";
  } else {

    // Get the "G" series out of the way first, otherwise it will interfere with the last regex!
    if (device_name.find("ATSAMG") != std::string::npos) {
      out = "SAMG_DFP";
    } else {
      pattern.assign("ATSAM([A-Z][0-9]+).*");
      std::regex_match(device_name, matches, pattern);
      if (!matches.empty()) {
        out = "SAM" + matches[1].str() + "_DFP";
      }
    }
  }

  return out;
}

static std::string resolve_at32uc_dfps(const std::string& device_name)
{
  std::string out;
  std::regex pattern("AT32UC3([A-Z]).*");
  std::smatch matches;

  std::regex_match(device_name, matches, pattern);
  if (!matches.empty()) {
    out = "UC3" + matches[1].str() + "_DFP";
  }

  if (out.empty()) {
    pattern.assign("ATUC[0-9]+([A-Z])([0-9])");
    std::regex_match(device_name, matches, pattern);

    if (!matches.empty()) {
      out = "UC" + matches[2].str() + matches[1].str() + "_DFP";
    }
  }

  return out;
}

static std::string resolve_xmega_dfps(const std::string& device_name)
{
  std::string out;
  std::regex pattern("ATxmega[0-9]+([A-Z]).*");
  std::smatch matches;

  std::regex_match(device_name, matches, pattern);
  if (!matches.empty()) {
    out = "XMEGA" + matches[1].str() + "_DFP";
  }

  return out;
}

std::string resolve_device_dfp_name(const std::string& device_name)
{
  Core core = resolve_core_from_name(device_name);
  std::string out = "";

  switch (core) {
    case Core::AT90mega:
    case Core::ATmega:
      out = "ATmega_DFP";
      break;

    case Core::ATtiny:
      // Special case handling for automotive attinies!
      if (cmutils::strings::to_uppercase(device_name).find("AUTO") != std::string::npos) {
        out = "ATautomotive_DFP";
      } else {
        out = "ATtiny_DFP";
      }
      break;

    case Core::ATautomotive:
      out = "ATautomotive_DFP";
      break;

    case Core::ATSAM:
      out = resolve_sam_dfps(device_name);
      break;

    case Core::AT32UC:
      out = resolve_at32uc_dfps(device_name);
      break;

    case Core::ATxmega:
      out = resolve_xmega_dfps(device_name);
      break;

    case Core::Unknown:
    default:
      break;
  }

  return out;
}

std::string get_max_packs_version(const std::string& path)
{
  std::string out;
  if (!std::filesystem::exists(path)) {
    return out;
  }

  std::vector<std::string> versions;

  for (const auto& entry : std::filesystem::directory_iterator(path)) {
    if (std::filesystem::is_directory(entry)) {
      std::string entry_name = entry.path().filename().string();

      // only accept folder versions like "12.34.56" with 2 dots
      if (std::count(entry_name.begin(), entry_name.end(), '.') == 2) {
        versions.push_back(entry_name);
      }
    }
  }

  // Computes weighted sum to discriminate the max folder version
  auto max_version = std::max_element(versions.begin(), versions.end(), [](const std::string& left, const std::string& right) {
    auto l = cmutils::strings::split(left, '.');
    auto r = cmutils::strings::split(right, '.');

    uint64_t lsum = 0;
    uint64_t rsum = 0;
    for (unsigned int i = 0; i < l.size(); i++) {
      lsum += (uint64_t)(std::stoi(l[i]) * pow(10, (l.size() - i)));
      rsum += (uint64_t)(std::stoi(r[i]) * pow(10, (l.size() - i)));
    }

    return lsum < rsum;
  });

  // Guard against cases where versions is empty
  if (max_version != versions.end()) {
    out = *max_version;
  }
  return out;
}

}
