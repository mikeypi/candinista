# project name and enable C++ support
project(candinista LANGUAGES C)

cmake_minimum_required(VERSION 3.25)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK REQUIRED IMPORTED_TARGET "gtk4")
pkg_check_modules(JSON REQUIRED IMPORTED_TARGET "json-c")

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
printjson.c
readjson.c
)

target_link_libraries(${PROJECT_NAME} ${GTK_LIBRARIES} ${JSON_LIBRARIES})

