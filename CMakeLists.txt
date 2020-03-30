cmake_minimum_required(VERSION 3.8.2)

project(Fart VERSION 1.0.0.0
  DESCRIPTION "Find And Replace Text command line utility."
  LANGUAGES C CXX)

set(libfart_target libfart)
set(fart_target fart)

list(APPEND libfart_source
  fart_shared.c)

list(APPEND libfart_public_headers
  fart_shared.h)

list(APPEND fart_source
  wildmat.c
  fart.cpp)

if(MSVC)
  set(FART_FILEDESCRIPTION "Find And Replace Text command line utility")
  set(FART_INTERNALNAME "${fart_target}")
  set(FART_ORIGINALFILENAME "${fart_target}.exe")
  set(FART_PRODUCTNAME "${fart_target}")
  configure_file("version.rc.in" "version.rc" @ONLY)
  list(APPEND fart_source
    "${CMAKE_CURRENT_BINARY_DIR}/version.rc")
endif()

add_library(${libfart_target} ${libfart_source})
add_executable(${fart_target} ${fart_source})
target_include_directories(${libfart_target} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${fart_target} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(${libfart_target} PROPERTIES
  PUBLIC_HEADER "${libfart_public_headers}")
target_link_libraries(${fart_target} ${libfart_target})


install(TARGETS ${libfart_target}
  PUBLIC_HEADER DESTINATION include
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)
install(TARGETS ${fart_target}
  LIBRARY DESTINATION bin
  ARCHIVE DESTINATION bin)
