
cmake_minimum_required(VERSION 2.8.0)

list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_SOURCE_DIR}/cmake")

project(KarboTickerApplet)

find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
  pkg_search_module(LIBCURL libcurl)
  pkg_search_module(LIBJANSSON jansson)
  pkg_search_module(LIBPANELAPPLE libpanelapplet-2.0)
endif()

include_directories(${LIBCURL_INCLUDE_DIRS} ${LIBJANSSON_INCLUDE_DIRS} ${LIBPANELAPPLE_INCLUDE_DIRS})

add_executable(${CMAKE_PROJECT_NAME} MarketsAPI.h MarketsAPI.cpp main.cpp)

target_link_libraries(${CMAKE_PROJECT_NAME} ${LIBCURL_LIBRARIES} ${LIBJANSSON_LIBRARIES} ${LIBPANELAPPLE_LIBRARIES})

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME} DESTINATION lib/bonobo/servers)
install(FILES KarboTickerApplet.server DESTINATION lib/KarboTickerApplet)

