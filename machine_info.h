#pragma once

#include <string>

bool machine_info_init();
bool machine_info_free();

std::string machine_info_uuid();
std::string machine_info_memory_serial0();

std::string machine_info_disks_controllerid();
std::string machine_info_disks_vserial();

std::string machine_info_bios_manufacturer();
std::string machine_info_bios_smbbversion();
std::string machine_info_bios_serial();

std::string machine_info_processor_name();
std::string machine_info_processor_id();
