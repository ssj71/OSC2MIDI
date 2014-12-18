cmake_minimum_required(VERSION 2.8)

add_subdirectory(src/)

install (FILES maps/*
  DESTINATION $ENV{HOME}/.osc2midi/
)
