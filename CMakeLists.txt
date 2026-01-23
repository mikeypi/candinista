# project name and enable C++ support
project(candinista LANGUAGES C)

cmake_minimum_required(VERSION 3.25)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK REQUIRED IMPORTED_TARGET "gtk4")
pkg_check_modules(JSON REQUIRED IMPORTED_TARGET "json-c")
pkg_check_modules(YAML REQUIRED IMPORTED_TARGET "yaml-0.1")
find_program(M4_EXECUTABLE m4 REQUIRED)
find_program(BP_EXECUTABLE blueprint-compiler REQUIRED)

include_directories(${GTK_INCLUDE_DIRS} ${JSON_INCLUDE_DIRS})


# we define the executable
add_executable(candinista "")

# and its sources
target_sources(
candinista
PRIVATE
candinista.c
datalogging.c
environ.c
interpolate.c
linear-panel.c
panel.c
radial-panel.c
yaml-loader.c
yaml-printer.c
sensor.c
cairo-misc.c
gtk-glue.c
info-panel.c
tpms-panel.c
units.c
d3-array.c
)

target_link_libraries(${PROJECT_NAME} ${GTK_LIBRARIES} ${JSON_LIBRARIES} ${YAML_LIBRARIES} m)

set(SRC ${CMAKE_CURRENT_SOURCE_DIR})
set(BIN ${CMAKE_CURRENT_BINARY_DIR})


add_executable(microsleep "")
#  and its sources
target_sources(
microsleep
PRIVATE
${CMAKE_CURRENT_SOURCE_DIR}/data-tools/microsleep.c
)

add_executable(decode "")
#  and its sources
target_sources(
decode
PRIVATE
${CMAKE_CURRENT_SOURCE_DIR}/data-tools/decode.c
)

add_executable(gensensordata "")
#  and its sources
target_sources(
gensensordata
PRIVATE
${CMAKE_CURRENT_SOURCE_DIR}/data-tools/gensensordata.c
)
