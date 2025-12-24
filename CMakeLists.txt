# project name and enable C++ support
project(candinista LANGUAGES C)

cmake_minimum_required(VERSION 3.25)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK REQUIRED IMPORTED_TARGET "gtk4")
pkg_check_modules(JSON REQUIRED IMPORTED_TARGET "json-c")
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
printjson.c
readjson.c
)

target_link_libraries(${PROJECT_NAME} ${GTK_LIBRARIES} ${JSON_LIBRARIES})

set(SRC ${CMAKE_CURRENT_SOURCE_DIR})
set(BIN ${CMAKE_CURRENT_BINARY_DIR})

set(M4_IN   ${SRC}/candinista.blp.m4)
set(BLP_OUT ${SRC}/candinista.blp)
set(UI_OUT  ${SRC}/candinista.ui)

add_custom_command(
    OUTPUT ${BLP_OUT}
    COMMAND ${M4_EXECUTABLE} ${M4_IN}
            > ${BLP_OUT}
    DEPENDS ${M4_IN}
    BYPRODUCTS ${BLP_OUT}
    COMMENT "m4: candinista.blp.m4 → candinista.blp"
    VERBATIM
)

add_custom_command(
    OUTPUT ${UI_OUT}
    COMMAND ${BP_EXECUTABLE} compile
            ${BLP_OUT}
            > ${UI_OUT}
    DEPENDS ${BLP_OUT}
    BYPRODUCTS ${UI_OUT}
    COMMENT "bp: candinista.blp → candinista.ui"
    VERBATIM
)

add_custom_target(generate_ui ALL
    DEPENDS ${UI_OUT}
)

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
