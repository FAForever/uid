include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

add_library(myeay32 SHARED
  myeay32.c
  myeay32.h
  myeay32.def
)
set_target_properties(myeay32 PROPERTIES
  LINKER_LANGUAGE C
  OUTPUT_NAME eay32
  RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}"
)

target_link_libraries(myeay32
  "${CMAKE_CURRENT_SOURCE_DIR}/libeay32md.dll"
)

add_custom_command(TARGET myeay32
  POST_BUILD
  COMMAND ${CMAKE_STRIP} -s "$<TARGET_FILE:myeay32>"
  COMMENT "Stripping executable $<TARGET_FILE_NAME:myeay32>"
  )

add_executable(test_compare_values
  ../machine_info_win.cpp
  test_compare_values.cpp
)
set_target_properties(test_compare_values PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}"
)
target_link_libraries(test_compare_values
  myeay32
  ${CMAKE_CURRENT_SOURCE_DIR}/uid.dll
  ${machine_info_libs}
  ${JSONCPP_LIBRARIES}
)
add_custom_command(TARGET test_compare_values
  POST_BUILD
  COMMAND ${CMAKE_STRIP} -s "$<TARGET_FILE:test_compare_values>"
  COMMENT "Stripping executable $<TARGET_FILE_NAME:test_compare_values>"
  )


file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/uid.dll DESTINATION ${PROJECT_BINARY_DIR}/)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/libeay32md.dll DESTINATION ${PROJECT_BINARY_DIR}/)
