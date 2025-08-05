cmake_minimum_required(VERSION 3.5)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules/")

project(osc2midi)

add_subdirectory(src/)

# file(MAKE_DIRECTORY /usr/share/osc2midi)
install (DIRECTORY maps/
  DESTINATION share/osc2midi
  USE_SOURCE_PERMISSIONS
)

install (DIRECTORY maps/touchosc/
  DESTINATION share/osc2midi/touchosc
  USE_SOURCE_PERMISSIONS
)
