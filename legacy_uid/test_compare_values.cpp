#include <iostream>
#include <limits>
#include <algorithm>
#include <stdlib.h>

#include <json/json.h>

#include "machine_info.h"
#include "myeay32.h"

#if defined(__cplusplus)
extern "C" {
#endif

extern char* uid(const char*, const char*);

#if defined(__cplusplus)
}
#endif

/* http://stackoverflow.com/questions/4643512 */
std::string replace_string(std::string subject,
                           const std::string& search,
                           const std::string& replace) {
    size_t pos = 0;
    while((pos = subject.find(search, pos)) != std::string::npos) {
         subject.replace(pos, search.length(), replace);
         pos += replace.length();
    }
    return subject;
}

void wait_for_return_pressed()
{
  std::cin.ignore( std::numeric_limits <std::streamsize> ::max(), '\n' );
}

int main(int argc, char *argv[])
{
  /* get the values of the new implementation */
  machine_info_init();
  std::string mi_uuid = machine_info_uuid();
  std::string mi_memory_serial0 = machine_info_memory_serial0();
  std::string mi_disks_controllerid = machine_info_disks_controllerid();
  std::string mi_bios_manufacturer = machine_info_bios_manufacturer();
  std::string mi_processor_name = machine_info_processor_name();
  std::string mi_processor_id = machine_info_processor_id();
  std::string mi_bios_smbbversion = machine_info_bios_smbbversion();
  std::string mi_bios_serial = machine_info_bios_serial();
  std::string mi_disks_vserial = machine_info_disks_vserial();
  machine_info_free();


  /* get the values using the myeay32 dll to leak the json string before encryption */
  std::string legacy_json_std_string;
  {
    char* legacy_json_string = static_cast<char*>(calloc(4096, 1));
    set_leak_ptr(legacy_json_string);
    uid("1234", "test.log");
    legacy_json_std_string = legacy_json_string;
    free(legacy_json_string);
  }
  /* do the backspace replacement */
  std::string legacy_json_std_string_replaced_backspace = replace_string(legacy_json_std_string, "\\","\\\\");

  Json::Value legacy_root;
  Json::Reader jsonReader;
  if (!jsonReader.parse(legacy_json_std_string_replaced_backspace,
                        legacy_root))
  {
    std::cerr << "Error parsing legacy JSON string: " << legacy_json_std_string_replaced_backspace
              << " error: " << jsonReader.getFormattedErrorMessages();
    wait_for_return_pressed();
    return 1;
  }

  bool hasError = false;
  if (legacy_root["machine"]["UUID"] != mi_uuid)
  {
    std::cerr << "machine_info_uuid doesn't match\n" <<
                 "legacy value '" << legacy_root["machine"]["UUID"] << "'\n" <<
                 "new value '" << mi_uuid << "'" << std::endl;
    hasError = true;
  }
  if (legacy_root["machine"]["mem_SerialNumber"] != mi_memory_serial0)
  {
    std::cerr << "machine_info_memory_serial0 doesn't match\n" <<
                 "\tlegacy value '" << legacy_root["machine"]["mem_SerialNumber"] << "'\n" <<
                 "\tnew value '" << mi_memory_serial0 << "'" << std::endl;
    hasError = true;
  }
  if (legacy_root["machine"]["DeviceID"] != mi_disks_controllerid)
  {
    std::cerr << "machine_info_disks_controllerid doesn't match\n" <<
                 "\tlegacy value '" << legacy_root["machine"]["DeviceID"] << "'\n" <<
                 "\tnew value '" << mi_disks_controllerid << "'" << std::endl;
    hasError = true;
  }
  if (legacy_root["machine"]["Manufacturer"] != mi_bios_manufacturer)
  {
    std::cerr << "machine_info_bios_manufacturer doesn't match\n" <<
                 "\tlegacy value '" << legacy_root["machine"]["Manufacturer"] << "'\n" <<
                 "\tnew value '" << mi_bios_manufacturer << "'" << std::endl;
    hasError = true;
  }
  if (legacy_root["machine"]["Name"] != mi_processor_name)
  {
    std::cerr << "machine_info_processor_name doesn't match\n" <<
                 "\tlegacy value '" << legacy_root["machine"]["Name"] << "'\n" <<
                 "\tnew value '" << mi_processor_name << "'" << std::endl;
    hasError = true;
  }
  if (legacy_root["machine"]["ProcessorId"] != mi_processor_id)
  {
    std::cerr << "machine_info_processor_id doesn't match\n" <<
                 "\tlegacy value '" << legacy_root["machine"]["ProcessorId"] << "'\n" <<
                 "\tnew value '" << mi_processor_id << "'" << std::endl;
    hasError = true;
  }
  if (legacy_root["machine"]["SMBIOSBIOSVersion"] != mi_bios_smbbversion)
  {
    std::cerr << "machine_info_bios_smbbversion doesn't match\n" <<
                 "\tlegacy value '" << legacy_root["machine"]["SMBIOSBIOSVersion"] << "'\n" <<
                 "\tnew value '" << mi_bios_smbbversion << "'" << std::endl;
    hasError = true;
  }
  if (legacy_root["machine"]["SerialNumber"] != mi_bios_serial)
  {
    std::cerr << "machine_info_bios_serial doesn't match\n" <<
                 "\tlegacy value '" << legacy_root["machine"]["SerialNumber"] << "'\n" <<
                 "\tnew value '" << mi_bios_serial << "'" << std::endl;
    hasError = true;
  }
  if (legacy_root["machine"]["VolumeSerialNumber"] != mi_disks_vserial)
  {
    std::cerr << "machine_info_disks_vserial doesn't match\n" <<
                 "\tlegacy value '" << legacy_root["machine"]["VolumeSerialNumber"] << "'\n" <<
                 "\tnew value '" << mi_disks_vserial << "'" << std::endl;
    hasError = true;
  }
  if (hasError)
  {
    std::cerr << "legacy_json_std_string:" << legacy_json_std_string << std::endl;
    std::cerr << "legacy_root:" << legacy_root << std::endl;
    std::cerr << "FAILED" << std::endl;
  }
  else
  {
    std::cout << "SUCCESS" << std::endl;
  }

  wait_for_return_pressed();
  return hasError;
}
