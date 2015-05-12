#CMake file for osc2midi

cmake_minimum_required(VERSION 2.8)

project (osc2midi)

set(CMAKE_C_FLAGS "-Wall -g")

# check for our various libraries
find_package(PkgConfig)
find_package(Jack REQUIRED)
pkg_check_modules(LO liblo)

include_directories (${LO_INCLUDE_DIRS} ${JACK_INCLUDE_DIRS})
link_directories (${LO_LIBRARY_DIRS} ${JACK_LIBRARY_DIRS})

# config libraries

add_executable(osc2midi
  pair.c
  oscserver.c
  jackdriver.c
  main.c
)


target_link_libraries(osc2midi ${LO_LIBRARIES} ${JACK_LIBRARIES} m)

# config install
install(TARGETS osc2midi
  DESTINATION bin
)

