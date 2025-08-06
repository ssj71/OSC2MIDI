#CMake file for osc2midi

cmake_minimum_required(VERSION 3.5)

project (osc2midi)

set(CMAKE_C_FLAGS "-Wall -g -DPREFIX='\"${CMAKE_INSTALL_PREFIX}\"'")

# check for our various libraries
find_package(PkgConfig)
find_package(Jack REQUIRED)
pkg_check_modules(LO liblo)

# Check liblo version for API compatibility
if(LO_FOUND)
    message(STATUS "Found liblo version: ${LO_VERSION}")
    # liblo 0.32 introduced breaking API changes to lo_method_handler
    if(LO_VERSION VERSION_GREATER_EQUAL "0.32")
        add_definitions(-DLIBLO_NEW_API)
        message(STATUS "Using new liblo API (>= 0.32)")
    else()
        message(STATUS "Using old liblo API (< 0.32)")
    endif()
endif()

include_directories (${LO_INCLUDE_DIRS} ${JACK_INCLUDE_DIRS})
link_directories (${LO_LIBRARY_DIRS} ${JACK_LIBRARY_DIRS})

# config libraries

add_executable(osc2midi
  pair.c
  hashtable.c
  ht_stuff.c
  oscserver.c
  jackmidi.c
  converter.c
  main.c
)


target_link_libraries(osc2midi ${LO_LIBRARIES} ${JACK_LIBRARIES} m)

# config install
install(TARGETS osc2midi
  DESTINATION bin
)

