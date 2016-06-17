#include "machine_info.h"

#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

std::string exec(const char* cmd) {
    char buffer[128];
    std::string result = "";
    std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (!feof(pipe.get())) {
        if (fgets(buffer, 128, pipe.get()) != NULL)
            result += buffer;
    }
    return result;
}

bool machine_info_init()
{
  return false;
}

bool machine_info_free()
{
  return false;
}

std::string machine_info_uuid()
{
  return "implementme";
}

std::string machine_info_memory_serial0()
{
  return "implementme";
}

std::string machine_info_disks_controllerid()
{
  return "implementme";
}
std::string machine_info_disks_vserial()
{
  return "implementme";
}

std::string machine_info_bios_manufacturer()
{
  return "implementme";
}
std::string machine_info_bios_smbbversion()
{
  return "implementme";
}
std::string machine_info_bios_serial()
{
  return "implementme";
}

std::string machine_info_processor_name()
{
  return "implementme";
}
std::string machine_info_processor_id()
{
  return "implementme";
}
