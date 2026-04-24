#include "machine_info.h"

#include <cstdio>
#include <memory>
#include <string>
#include <regex>
#include <algorithm>
#include <cctype>

std::string exc_value("invalid");

std::string& rtrim(std::string &s)
{
  s.erase(std::find_if(s.rbegin(),
                       s.rend(),
                       [](unsigned char ch) { return !std::isspace(ch); }).base(),
          s.end());
  return s;
}

std::string exec(const char* cmd)
{
  char buffer[128];
  std::string result = "";
  std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
  if (!pipe)
  {
    return exc_value;
  }
  while (!feof(pipe.get()))
  {
      if (fgets(buffer, 128, pipe.get()) != NULL)
          result += buffer;
  }
  rtrim(result);
  if (result.empty())
  {
    return exc_value;
  }
  return result;
}

std::string match_regex(std::string const& input,
                        char const* r_string)
{
  std::regex r(r_string);
  std::smatch sm;
  if (std::regex_search(input, sm, r))
  {
    return sm[1];
  }
  return exc_value;
}

bool machine_info_init()
{
  return true;
}

bool machine_info_free()
{
  return false;
}

std::string machine_info_uuid()
{
  return exec("ioreg -rd1 -c IOPlatformExpertDevice "
              "| awk '/IOPlatformUUID/ { gsub(/\"/, \"\", $3); print $3 }'");
}

std::string machine_info_model()
{
  return exec("sysctl -n hw.model");
}

std::string machine_info_manufacturer()
{
  return std::string("Apple Inc.");
}

// system_profiler SPDisplaysDataType can take several seconds; cache its
// output so the width and height accessors share a single invocation.
static std::string const& display_data()
{
  static std::string const data = exec("system_profiler SPDisplaysDataType");
  return data;
}

std::string machine_info_display_width()
{
  return match_regex(display_data(), "Resolution:\\s+(\\d+)\\s+x\\s+\\d+");
}

std::string machine_info_display_height()
{
  return match_regex(display_data(), "Resolution:\\s+\\d+\\s+x\\s+(\\d+)");
}

std::string machine_info_memory_serial0()
{
  return exc_value;
}

std::string machine_info_motherboard_vendor()
{
  return std::string("Apple Inc.");
}

std::string machine_info_motherboard_name()
{
  return exec("sysctl -n hw.model");
}

std::string machine_info_disks_controllerid()
{
  return exc_value;
}

std::string machine_info_disks_vserial()
{
  // Anchor to the exact "UUID" key — ioreg output for IOMedia can
  // include other keys containing the substring UUID (e.g.
  // "VolGroupUUID", "Content Hint UUID"), and an unanchored /UUID/
  // would match those first on any machine where they appear above
  // the plain "UUID" line.
  return exec("ioreg -rd1 -c IOMedia "
              "| awk '/\"UUID\" =/ { gsub(/\"/, \"\", $3); print $3; exit }'");
}

std::string machine_info_bios_manufacturer()
{
  return std::string("Apple Inc.");
}

std::string machine_info_bios_smbbversion()
{
  return machine_info_bios_version();
}

std::string machine_info_bios_serial()
{
  return exec("ioreg -rd1 -c IOPlatformExpertDevice "
              "| awk '/IOPlatformSerialNumber/ { gsub(/\"/, \"\", $3); print $3 }'");
}

std::string machine_info_bios_description()
{
  return exc_value;
}

std::string machine_info_bios_date()
{
  return exc_value;
}

std::string machine_info_bios_version()
{
  // Apple Silicon has no BIOS; the equivalent is the BootROM /
  // System Firmware Version. This is stable across macOS updates
  // (only changes on firmware updates), so it gives a durable
  // fingerprint contribution — unlike kern.osrelease, which changes
  // with every OS point release.
  return exec("system_profiler SPHardwareDataType "
              "| awk -F': ' '/System Firmware Version/ {print $2; exit}'");
}

std::string machine_info_processor_name()
{
  return exec("sysctl -n machdep.cpu.brand_string");
}

std::string machine_info_processor_id()
{
  return exc_value;
}

std::string machine_info_os_type()
{
  return exec("uname -s");
}

std::string machine_info_os_version()
{
  return exec("sw_vers -productVersion");
}
