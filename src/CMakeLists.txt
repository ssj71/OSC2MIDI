#CMake file for osc2midi

cmake_minimum_required(VERSION 2.8)

# set(PLUGIN "stuck")
# set(PLUGDIR "Stuck")
project (osc2midi)

# set(LV2_INSTALL_DIR /usr/lib/lv2/${PLUGIN}.lv2 ) # CACHE PATH "Specifies where the LV2 libraries should be installed")
set(CMAKE_C_FLAGS "-Wall -g")

# check for our various libraries
find_package(PkgConfig)
pkg_check_modules(LO lo)
pkg_check_modules(JACK jack)
# pkg_check_modules(LV2 REQUIRED lv2>=.0.1.0) 

include_directories (${LO_INCLUDE_DIRS} ${JACK_INCLUDE_DIRS})
link_directories (${LO_LIBRARY_DIRS} ${JACK_LIBRARY_DIRS})

# config libraries

add_library(${PLUGIN} SHARED
  stuck.c
  stuck.h
  rms_calc.c
  rms_calc.h
)


target_link_libraries(${PLUGIN} ${LO_LIBRARIES} ${JACK_LIBRARIES} m)
target_link_libraries(${PLUGIN}_ui ${LV2_LIBRARIES} ${NTK_LIBRARIES} ${CAIRO_LIBRARIES} m)

# config install
install(TARGETS ${PLUGIN} ${PLUGIN}_ui
  LIBRARY
  DESTINATION ${LV2_INSTALL_DIR}
)

install (FILES ${PLUGIN}.lv2/manifest.ttl ${PLUGIN}.lv2/${PLUGIN}.ttl 
  DESTINATION ${LV2_INSTALL_DIR}
)
