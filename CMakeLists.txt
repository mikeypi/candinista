cmake_minimum_required(VERSION 3.25)

project(candinista LANGUAGES C)

# -----------------------------
# Tooling
# -----------------------------
find_package(PkgConfig REQUIRED)

pkg_check_modules(GTK  REQUIRED IMPORTED_TARGET gtk4)
pkg_check_modules(YAML REQUIRED IMPORTED_TARGET yaml-0.1)


# -----------------------------
# Cairo / UI shared library
# -----------------------------
add_library(candinsta_shared STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/loader/yaml-loader.c
    ${CMAKE_CURRENT_SOURCE_DIR}/loader/yaml-printer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/loader/d3-array.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/info-panel.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/cairo-misc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/linear-panel.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/gps-panel.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/radial-panel.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/tpms-panel.c
    ${CMAKE_CURRENT_SOURCE_DIR}/misc/datalogging.c
    ${CMAKE_CURRENT_SOURCE_DIR}/misc/environ.c
    ${CMAKE_CURRENT_SOURCE_DIR}/misc/interpolate.c
    ${CMAKE_CURRENT_SOURCE_DIR}/misc/panel.c
    ${CMAKE_CURRENT_SOURCE_DIR}/misc/sensor.c
    ${CMAKE_CURRENT_SOURCE_DIR}/misc/gtk-glue.c
    ${CMAKE_CURRENT_SOURCE_DIR}/misc/units.c
    # add other shared UI/cairo files here if you want
)

target_compile_options(candinsta_shared
    PRIVATE
        $<$<C_COMPILER_ID:GNU,Clang>:-Wall -Wextra>
        $<$<C_COMPILER_ID:MSVC>:/W4>
)

# If info-panel.c includes <gtk/gtk.h>, it MUST get GTK include dirs.
# Linking to the imported target carries include dirs + compile defs.
target_link_libraries(candinsta_shared
    PUBLIC
        PkgConfig::GTK
)

target_include_directories(candinsta_shared
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/ui
	${CMAKE_CURRENT_SOURCE_DIR}/loader
	${CMAKE_CURRENT_SOURCE_DIR}/misc
)

# libm only on Unix-like platforms
if(UNIX)
  target_link_libraries(candinsta_shared PUBLIC m)
endif()

# -----------------------------
# candinista executable
# -----------------------------
add_executable(candinista
    candinista.c
)

target_compile_options(candinista
    PRIVATE
        $<$<C_COMPILER_ID:GNU,Clang>:-Wall -Wextra>
        $<$<C_COMPILER_ID:MSVC>:/W4>
)

target_link_libraries(candinista
    PRIVATE
        candinsta_shared
        PkgConfig::GTK
        PkgConfig::YAML
)

# -----------------------------
# eyecandy executable
# -----------------------------
add_executable(eyecandy
    ${CMAKE_CURRENT_SOURCE_DIR}/eyecandy/eyecandy.c
)

target_compile_options(eyecandy
    PRIVATE
        $<$<C_COMPILER_ID:GNU,Clang>:-Wall -Wextra>
        $<$<C_COMPILER_ID:MSVC>:/W4>
)

target_link_libraries(eyecandy
    PRIVATE
        candinsta_shared
        PkgConfig::GTK
        PkgConfig::YAML
)

# -----------------------------
# data-tools
# -----------------------------
add_executable(microsleep
    ${CMAKE_CURRENT_SOURCE_DIR}/data-tools/microsleep.c
)

add_executable(decode
    ${CMAKE_CURRENT_SOURCE_DIR}/data-tools/decode.c
)

add_executable(gensensordata
    ${CMAKE_CURRENT_SOURCE_DIR}/data-tools/gensensordata.c
)
