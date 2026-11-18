#include "machine_info.h"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <regex>
#include <string>

#include <sys/system_properties.h>

/* Android gives apps no hardware serials: since Android 10 the serial
   number and the IMEI need READ_PRIVILEGED_PHONE_STATE, which only system
   apps hold, and the MAC address is hidden. The device fields are the
   system properties android.os.Build reports. The one per-device id, the
   app-scoped Settings.Secure.ANDROID_ID, is only reachable from Java, so the
   app running faf-uid passes it in FAF_UID_ANDROID_ID. */

namespace
{

std::string const exc_value("invalid");

std::string& rtrim(std::string &s)
{
  s.erase(std::find_if(s.rbegin(),
                       s.rend(),
                       [](unsigned char ch) { return !std::isspace(ch); }).base(),
          s.end());
  return s;
}

std::string read_file(std::string const& filename)
{
  std::ifstream ifs(filename);
  if (!ifs.good())
  {
    return exc_value;
  }
  std::string content((std::istreambuf_iterator<char>(ifs)),
                       std::istreambuf_iterator<char>());
  rtrim(content);
  return content;
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

std::string property(char const* name)
{
  std::string value;
  prop_info const* info = __system_property_find(name);
  if (info != nullptr)
  {
    __system_property_read_callback(info,
                                    [](void* cookie, char const*, char const* v, uint32_t)
                                    {
                                      *static_cast<std::string*>(cookie) = v;
                                    },
                                    &value);
  }
  if (value.empty())
  {
    return exc_value;
  }
  return value;
}

}

bool machine_info_init()
{
  return true;
}

bool machine_info_free()
{
  return true;
}

std::string machine_info_uuid()
{
  char const* android_id = std::getenv("FAF_UID_ANDROID_ID");
  if (android_id == nullptr || *android_id == '\0')
  {
    return exc_value;
  }
  return android_id;
}

std::string machine_info_model()
{
  return property("ro.product.model");
}

std::string machine_info_manufacturer()
{
  return property("ro.product.manufacturer");
}

// Only Java knows the display, and it is the same for every phone of a model.
std::string machine_info_display_width()
{
  return exc_value;
}

std::string machine_info_display_height()
{
  return exc_value;
}

std::string machine_info_memory_serial0()
{
  return exc_value;
}

std::string machine_info_motherboard_vendor()
{
  return property("ro.product.brand");
}

std::string machine_info_motherboard_name()
{
  return property("ro.product.board");
}

std::string machine_info_disks_controllerid()
{
  return exc_value;
}

std::string machine_info_disks_vserial()
{
  return exc_value;
}

std::string machine_info_bios_manufacturer()
{
  return exc_value;
}

std::string machine_info_bios_smbbversion()
{
  return machine_info_bios_version();
}

std::string machine_info_bios_serial()
{
  return exc_value;
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
  // The closest thing, ro.bootloader, changes with every system update.
  return exc_value;
}

std::string machine_info_processor_name()
{
  // Build.SOC_MODEL, set since Android 12
  std::string soc_model = property("ro.soc.model");
  if (soc_model != exc_value)
  {
    return soc_model;
  }
  std::string cpuinfo = read_file("/proc/cpuinfo");
  if (cpuinfo == exc_value)
  {
    return exc_value;
  }
  // x86 kernels name the processor, older ARM vendor kernels the SoC
  std::string name = match_regex(cpuinfo, "model name\\s*: (.*)");
  if (name == exc_value)
  {
    name = match_regex(cpuinfo, "Hardware\\s*: (.*)");
  }
  return name;
}

std::string machine_info_processor_id()
{
  return exc_value;
}

std::string machine_info_os_type()
{
  return "Android";
}

std::string machine_info_os_version()
{
  return property("ro.build.version.release");
}
