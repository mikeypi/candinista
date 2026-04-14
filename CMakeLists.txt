cmake_minimum_required(VERSION 3.25)

project(candinista LANGUAGES C)

# -----------------------------
# Tooling
# -----------------------------
find_package(PkgConfig REQUIRED)

pkg_check_modules(GTK  REQUIRED IMPORTED_TARGET gtk4)
pkg_check_modules(YAML REQUIRED IMPORTED_TARGET yaml-0.1)


# -----------------------------
# Shared library
# -----------------------------
add_library(candinsta_shared STATIC
    # Config / serialization
    ${CMAKE_CURRENT_SOURCE_DIR}/config/yaml-loader.c
    ${CMAKE_CURRENT_SOURCE_DIR}/config/yaml-printer.c

    # UI / Cairo panel renderers
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/cairo-misc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/gtk-glue.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/info-panel.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/linear-panel.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/gps-panel.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/radial-panel.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/tpms-panel.c

    # Core data model & utilities
    ${CMAKE_CURRENT_SOURCE_DIR}/core/sensor.c
    ${CMAKE_CURRENT_SOURCE_DIR}/core/panel.c
    ${CMAKE_CURRENT_SOURCE_DIR}/core/units.c
    ${CMAKE_CURRENT_SOURCE_DIR}/core/interpolate.c
    ${CMAKE_CURRENT_SOURCE_DIR}/core/datalogging.c
    ${CMAKE_CURRENT_SOURCE_DIR}/core/environ.c
)

target_compile_options(candinsta_shared
    PRIVATE
        $<$<C_COMPILER_ID:GNU,Clang>:-Wall -Wextra>
        $<$<C_COMPILER_ID:MSVC>:/W4>
)

target_link_libraries(candinsta_shared
    PUBLIC
        PkgConfig::GTK
)

target_include_directories(candinsta_shared
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/config
        ${CMAKE_CURRENT_SOURCE_DIR}/core
        ${CMAKE_CURRENT_SOURCE_DIR}/ui
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
