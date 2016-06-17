cmake_minimum_required(VERSION 2.8)

project(faf_uid CXX)

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

set(CMAKE_CXX_STANDARD 11)

if (WIN32)
  set(machine_info_src machine_info_win.cpp)
  set(machine_info_libs
        wbemuuid.lib
      )
else()
  set(machine_info_src machine_info_linux.cpp)
endif()

set(UID_PUBKEY_BYTES
  "130,99,238,192,232,47,187,99,222,116,140,101,233,231,57,188,204,204,187,241,173,147,88,60,217,7,80,217"
  CACHE STRING
  "the public key used for encryption"
  )

add_definitions(-DUID_PUBKEY_BYTES="${UID_PUBKEY_BYTES}")

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}-static")

add_executable(uid
  uid.cpp
  ${machine_info_src}
  )

target_link_libraries(uid
  ${CRYPTOPP_LIBRARIES}
  ${JSONCPP_LIBRARIES}
  ${machine_info_libs}
  )
