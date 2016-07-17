#include "machine_info.h"

#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <fstream>
#include <regex>
#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>

#include <json/json.h>

std::string exc_value("invalid");

std::string& rtrim(std::string &s)
{
  s.erase(std::find_if(s.rbegin(),
                       s.rend(),
                       std::not1(std::ptr_fun<int, int>(std::isspace))).base(),
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
  return result;
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
  return read_file("/var/lib/dbus/machine-id");
}

std::string machine_info_model()
{
  return read_file("/sys/class/dmi/id/product_name");
}

std::string machine_info_manufacturer()
{
  return read_file("/sys/class/dmi/id/sys_vendor");
}

std::string machine_info_display_width()
{
  std::string xrandr = exec("xrandr");
  return match_regex(xrandr, "current (\\d+) x \\d+");
}

std::string machine_info_display_height()
{
  std::string xrandr = exec("xrandr");
  return match_regex(xrandr, "current \\d+ x (\\d+)");
}

std::string machine_info_memory_serial0()
{
  return exc_value;
}

std::string machine_info_motherboard_vendor()
{
  std::string result = read_file("/sys/class/dmi/id/board_vendor");
  if (result == exc_value)
  {
    return read_file("/sys/class/virtual/dmi/id/board_vendor");
  }
  return result;
}

std::string machine_info_motherboard_name()
{
  std::string result = read_file("/sys/class/dmi/id/board_name");
  if (result == exc_value)
  {
    return read_file("/sys/class/virtual/dmi/id/board_name");
  }
  return result;
}

std::string machine_info_disks_controllerid()
{
  std::string lspci = exec("lspci -n");
  return match_regex(lspci, "..:..\\..\\s0106:\\s(....:....)\\s");
}


bool is_root_device(Json::Value const& d)
{
  if (!d["mountpoint"].isNull())
  {
    return d["mountpoint"].asString() == "/";
  }
  else if (!d["children"].isNull())
  {
    bool result = false;
    for(auto c: d["children"])
    {
      result |= is_root_device(c);
    }
    return result;
  }
  return false;
}

std::string machine_info_disks_vserial()
{
  std::string lsblk_json_string = exec("lsblk --json -o SERIAL,NAME,MOUNTPOINT");
  Json::Value lsblk_json;
  Json::Reader jsonReader;
  if (!jsonReader.parse(lsblk_json_string,
                        lsblk_json))
  {
    return exc_value;
  }
  for(auto d: lsblk_json["blockdevices"])
  {
    if (is_root_device(d))
    {
      return d["serial"].asString();
    }
  }
  return exc_value;
}

std::string machine_info_bios_manufacturer()
{
  std::string result = read_file("/sys/class/dmi/id/bios_vendor");
  if (result == exc_value)
  {
    return read_file("/sys/class/virtual/dmi/id/bios_vendor");
  }
  return result;
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
  std::string result = read_file("/sys/class/dmi/id/bios_date");
  if (result == exc_value)
  {
    return read_file("/sys/class/virtual/dmi/id/bios_date");
  }
  return result;
}

std::string machine_info_bios_version()
{
  std::string result = read_file("/sys/class/dmi/id/bios_version");
  if (result == exc_value)
  {
    return read_file("/sys/class/virtual/dmi/id/bios_version");
  }
  return result;
}

std::string machine_info_processor_name()
{
  std::string cpuinfo = read_file("/proc/cpuinfo");
  if (cpuinfo == exc_value)
  {
    return exc_value;
  }
  return match_regex(cpuinfo, "model name\\s*: (.*)");
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
  return exec("uname -r");
}
