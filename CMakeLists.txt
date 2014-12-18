cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules/")

add_subdirectory(src/)

file(MAKE_DIRECTORY $ENV{HOME}/.osc2midi)
install (DIRECTORY maps/
  DESTINATION $ENV{HOME}/.osc2midi
  USE_SOURCE_PERMISSIONS
)
