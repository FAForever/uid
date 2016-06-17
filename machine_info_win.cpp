#include "machine_info.h"

#include <string>
#include <sstream>
#include <vector>

#include <unicode/unistr.h>

#define _WIN32_DCOM
#include <iostream>
#include <windows.h>
#include <wbemidl.h>
#include <comdef.h>

IWbemServices *pSvc(NULL);
IWbemLocator *pLoc(NULL);

bool machine_info_init()
{
  /* https://msdn.microsoft.com/en-us/library/aa390423(v=vs.85).aspx */
  HRESULT hres;

  // Step 1: --------------------------------------------------
  // Initialize COM. ------------------------------------------
  hres =  CoInitializeEx(0, COINIT_MULTITHREADED);
  if (FAILED(hres))
  {
      std::cerr << "Failed to initialize COM library. Error code = 0x"
                << std::hex
                << hres
                << std::endl;
      return false;
  }
  // Step 2: --------------------------------------------------
  // Set general COM security levels --------------------------

  hres =  CoInitializeSecurity(NULL,
                               -1,                          // COM authentication
                               NULL,                        // Authentication services
                               NULL,                        // Reserved
                               RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication
                               RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation
                               NULL,                        // Authentication info
                               EOAC_NONE,                   // Additional capabilities
                               NULL                         // Reserved
                               );

  if (FAILED(hres))
  {
    std::cerr << "Failed to initialize security. Error code = 0x"
              << std::hex
              << hres
              << std::endl;
    CoUninitialize();
    return NULL;                    // Program has failed.
  }

  // Step 3: ---------------------------------------------------
  // Obtain the initial locator to WMI -------------------------

  IWbemLocator *pLoc = NULL;

  hres = CoCreateInstance(CLSID_WbemLocator,
                          NULL,
                          CLSCTX_ALL,
                          IID_PPV_ARGS(&pLoc));

  if (FAILED(hres))
  {
    std::cerr << "Failed to create IWbemLocator object."
        << " Err code = 0x"
        << std::hex
        << hres
        << std::endl;
    pLoc = NULL;
    return false;
  }

  // Step 4: -----------------------------------------------------
  // Connect to WMI through the IWbemLocator::ConnectServer method

  // Connect to the root\cimv2 namespace with
  // the current user and obtain pointer pSvc
  // to make IWbemServices calls.
  //BSTR wmiNamespace(L"root\\cimv2");
  hres = pLoc->ConnectServer(BSTR(L"root\\cimv2"),          // Object path of WMI namespace
                             NULL,                          // User name. NULL = current user
                             NULL,                          // User password. NULL = current
                             NULL,                          // Locale. NULL indicates current
                             WBEM_FLAG_CONNECT_USE_MAX_WAIT,// Security flags.
                             NULL,                          // Authority (for example, Kerberos)
                             NULL,                          // Context object
                             &pSvc                          // pointer to IWbemServices proxy
                             );

  if (FAILED(hres))
  {
    std::cerr << "Could not connect. Error code = 0x"
              << std::hex
              << hres
              << std::endl;
    pSvc = NULL;
    return false;
  }

  // Step 5: --------------------------------------------------
  // Set security levels on the proxy -------------------------

  hres = CoSetProxyBlanket(pSvc,                        // Indicates the proxy to set
                           RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
                           RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
                           NULL,                        // Server principal name
                           RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx
                           RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
                           NULL,                        // client identity
                           EOAC_NONE                    // proxy capabilities
                        );

  if (FAILED(hres))
  {
    std::cerr << "Could not set proxy blanket. Error code = 0x"
              << std::hex
              << hres
              << std::endl;
    machine_info_free();
    return false;
  }
  return true;
}

bool machine_info_free()
{
  if (pSvc != NULL)
  {
    pSvc->Release();
    pSvc = NULL;
  }
  if (pLoc != NULL)
  {
    pLoc->Release();
    pLoc = NULL;
  }
  CoUninitialize();
  return true;
}

std::vector<std::string> read_wmi_values(std::wstring const& table,
                                         std::wstring const& property,
                                         std::wstring const& filter = std::wstring())
{
  std::vector<std::string> result;
  if (pSvc == NULL)
  {
    std::cerr << "WMI uninitilized. Call machine_info_init() first" << std::endl;
    return result;
  }

  std::wstring query(L"SELECT * FROM ");
  query += table;

  if (filter.size())
  {
    query += L" WHERE ";
    query += filter;
  }

  IEnumWbemClassObject* pEnumerator = NULL;

  HRESULT hres;
  hres = pSvc->ExecQuery(
      BSTR(L"WQL"),
      (wchar_t*)query.c_str(),
      WBEM_FLAG_FORWARD_ONLY,
      NULL,
      &pEnumerator);

  if (FAILED(hres))
  {
      std::wcerr << "Query for "
                << table
                << "."
                << property.c_str()
                << " failed."
                << " Error code = 0x"
                << std::hex
                << hres
                << std::endl;
      return result;               // Program has failed.
  }

  // Step 7: -------------------------------------------------
  // Get the data from the query in step 6 -------------------

  IWbemClassObject *pclsObj = NULL;
  int numElems;

  while((hres = pEnumerator->Next(WBEM_INFINITE,
                                  1,
                                  &pclsObj,
                                  (ULONG*)&numElems)) != WBEM_S_FALSE)
  {
      VARIANT vtProp;
      VariantInit(&vtProp);

      // Get the value of the Name property
      hres = pclsObj->Get(property.c_str(), 0, &vtProp, 0, 0);

      if (FAILED(hres))
      {
          std::wcerr << "Getting property "
                     << table
                     << "."
                     << property.c_str()
                     << " failed."
                     << " Error code = 0x"
                     << std::hex
                     << hres
                     << " query "
                     << query.c_str()
                     << std::endl;
          break;
      }

      if (vtProp.vt == VT_BSTR)
      {
        std::string propString;
        UnicodeString(vtProp.bstrVal).toUTF8String(propString);
        result.push_back(propString);
      }
      else if (vtProp.vt == VT_I4)
      {
        result.push_back(static_cast< std::ostringstream & >(( std::ostringstream() << std::dec << vtProp.iVal ) ).str());
      }

      VariantClear(&vtProp);

      pclsObj->Release();
  }
  pEnumerator->Release();
  return result;
}

std::string wmi_value(std::wstring const& table,
                      std::wstring const& property,
                      std::wstring const& filter = std::wstring())
{
  std::vector<std::string> values = read_wmi_values(table,
                                property,
                                filter);

  if (values.size())
  {
    return values.front();
  }
  return "invalid";
}

std::string machine_info_uuid()
{
  return wmi_value(L"Win32_ComputerSystemProduct",
                   L"UUID");
}

std::string machine_info_memory_serial0()
{
  return wmi_value(L"Win32_PhysicalMemory",
                   L"SerialNumber");
}

std::string machine_info_disks_controllerid()
{
  return wmi_value(L"Win32_IDEController",
                   L"DeviceID");
}

std::string machine_info_disks_vserial()
{
  return wmi_value(L"Win32_LogicalDisk",
                   L"VolumeSerialNumber",
                   L"DeviceID=\"C:\"");
}

std::string machine_info_bios_manufacturer()
{
  return wmi_value(L"Win32_BIOS",
                   L"Manufacturer");
}
std::string machine_info_bios_smbbversion()
{
  return wmi_value(L"Win32_BIOS",
                   L"SMBIOSBIOSVersion");
}
std::string machine_info_bios_serial()
{
  return wmi_value(L"Win32_BIOS",
                   L"SerialNumber");
}

std::string machine_info_processor_name()
{
  return wmi_value(L"Win32_Processor",
                   L"Name");
}
std::string machine_info_processor_id()
{
  return wmi_value(L"Win32_Processor",
                   L"ProcessorId");
}
