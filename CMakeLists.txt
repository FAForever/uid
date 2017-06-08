cmake_minimum_required(VERSION 2.8)

project(faf-uid CXX C)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" UID_COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" UID_COMPILER_SUPPORTS_CXX0X)
if(UID_COMPILER_SUPPORTS_CXX11)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(UID_COMPILER_SUPPORTS_CXX0X)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
  message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

if(NOT DEFINED JSONCPP_LIBRARIES)
  if(NOT PKGCONFIG_FOUND)
    find_package(PkgConfig REQUIRED)
  endif()
  pkg_search_module(JSONCPP REQUIRED jsoncpp)
endif()

include_directories(${JSONCPP_INCLUDE_DIRS})

if(NOT DEFINED CRYPTOPP_LIBRARIES)
  if(NOT PKGCONFIG_FOUND)
    find_package(PkgConfig REQUIRED)
  endif()
  pkg_search_module(CRYPTOPP REQUIRED cryptopp)
endif()

include_directories(${CRYPTOPP_INCLUDE_DIRS})

if (WIN32)
  if(NOT PKGCONFIG_FOUND)
    find_package(PkgConfig REQUIRED)
  endif()
  pkg_search_module(ICU REQUIRED icu-uc)
  include_directories(${ICU_INCLUDE_DIRS})
  set(machine_info_src machine_info_win.cpp)
  set(machine_info_libs
    ${ICU_LIBRARIES}
    wbemuuid.lib
      )
else()
  set(machine_info_src machine_info_linux.cpp)
endif()

if(NOT DEFINED UID_PUBKEY_BYTES)
  message(WARNING "You need to set UID_PUBKEY_BYTES to the output of encode_openssl_modulus.py. Using example public key.")
  set(UID_PUBKEY_BYTES
    "200,172,159,159,117,211,197,121,7,80,96,139,82,253,240,67,219,77,244,65,25,119,3,147,242,142,113,95,120,226,30,104,158,211,48,73,96,59,85,198,183,199,146,127,140,87,183,110,75,173,39,218,35,146,181,21,115,29,238,23,204,109,15,252,99,204,103,130,138,78,12,7,65,20,247,29,195,136,20,53,200,221,58,114,11,170,65,151,100,61,139,170,244,158,7,192,99,91,142,217,139,253,106,198,180,112,173,49,106,90,121,163,2,24,206,176,198,187,35,37,111,218,197,202,247,139,30,126,152,38,34,73,25,199,10,194,12,196,144,195,98,48,149,14,219,39,182,154,73,246,96,81,152,95,163,251,8,117,35,226,61,16,164,190,128,239,187,122,78,102,209,233,11,126,80,71,187,78,239,28,48,175,91,51,100,83,165,203,222,119,117,138,82,131,199,90,134,250,62,51,231,180,158,11,109,138,75,37,221,145,184,14,177,203,192,191,48,25,159,137,191,252,5,174,209,207,247,198,32,56,152,65,134,251,180,147,36,250,95,50,253,103,240,3,100,211,86,117,5,63,205,61,176,76,48,209"
    CACHE STRING
    "the public key used for encryption"
    )
endif()

add_definitions(-DUID_PUBKEY_BYTES=${UID_PUBKEY_BYTES})

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")

add_executable(faf-uid
  uid.cpp
  ${machine_info_src}
  README.md
  .travis.yml
  )

# Disable console window popping up when running the exe from Python
# Note: this also disables the output running the exe from cmd
if (WIN32)
  set_target_properties(faf-uid PROPERTIES LINK_FLAGS -mwindows)
endif()

target_link_libraries(faf-uid
  ${CRYPTOPP_LIBRARIES}
  ${JSONCPP_LIBRARIES}
  ${machine_info_libs}
  )

# Strip exe to reduce file size
add_custom_command(TARGET faf-uid
  POST_BUILD
  COMMAND ${CMAKE_STRIP} -s "$<TARGET_FILE:faf-uid>"
  COMMENT "Stripping executable $<TARGET_FILE_NAME:faf-uid>"
  )

if (WIN32)
  option(UID_SKIP_LEGACY "Do not build the legacy test" OFF)
  if(NOT UID_SKIP_LEGACY)
    add_subdirectory(legacy_uid)
  endif()
endif()
