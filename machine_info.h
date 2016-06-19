#pragma once

#include <string>

bool machine_info_init();
bool machine_info_free();

std::string machine_info_uuid();
std::string machine_info_model();
std::string machine_info_manufacturer();

std::string machine_info_desktop_width();
std::string machine_info_desktop_height();

std::string machine_info_memory_serial0();

std::string machine_info_motherboard_vendor();
std::string machine_info_motherboard_name();

std::string machine_info_disks_controllerid();
std::string machine_info_disks_vserial();

std::string machine_info_bios_manufacturer();
std::string machine_info_bios_smbbversion();
std::string machine_info_bios_serial();
std::string machine_info_bios_description();
std::string machine_info_bios_date();
std::string machine_info_bios_version();

std::string machine_info_processor_name();
std::string machine_info_processor_id();

std::string machine_info_os_type();
std::string machine_info_os_version();
