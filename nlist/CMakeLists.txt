cmake_minimum_required (VERSION 2.8)

project (nlist)

include_directories ("${PROJECT_SOURCE_DIR}/../cctools-886/include")

add_executable(nlist nlist.c)

target_link_libraries (nlist "${PROJECT_SOURCE_DIR}/../cctools-886/libstuff/libstuff_p.a")
