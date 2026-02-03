cmake_minimum_required(VERSION 3.25)

project(candinista LANGUAGES C)

# -----------------------------
# Tooling
# -----------------------------
find_package(PkgConfig REQUIRED)

pkg_check_modules(GTK  REQUIRED IMPORTED_TARGET gtk4)
pkg_check_modules(YAML REQUIRED IMPORTED_TARGET yaml-0.1)

# -----------------------------
# YAML Shared library
# -----------------------------
add_library(yaml_shared STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/yaml/yaml-loader.c
    ${CMAKE_CURRENT_SOURCE_DIR}/yaml/yaml-printer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/yaml/d3-array.c
)

target_compile_options(yaml_shared
    PRIVATE
        $<$<C_COMPILER_ID:GNU,Clang>:-Wall -Wextra>
        $<$<C_COMPILER_ID:MSVC>:/W4>
)

# Let anything using yaml_shared also see headers in ./yaml
target_include_directories(yaml_shared
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/yaml
	${CMAKE_CURRENT_SOURCE_DIR}/cairo
)

target_link_libraries(yaml_shared
    PUBLIC
        PkgConfig::GTK    
        PkgConfig::YAML
)

# -----------------------------
# Cairo / UI shared library
# -----------------------------
add_library(cairo_shared STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/cairo/info-panel.c
    ${CMAKE_CURRENT_SOURCE_DIR}/cairo/cairo-misc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/cairo/linear-panel.c
    ${CMAKE_CURRENT_SOURCE_DIR}/cairo/gps-panel.c
    ${CMAKE_CURRENT_SOURCE_DIR}/cairo/radial-panel.c
    ${CMAKE_CURRENT_SOURCE_DIR}/cairo/tpms-panel.c
    # add other shared UI/cairo files here if you want
)

target_compile_options(cairo_shared
    PRIVATE
        $<$<C_COMPILER_ID:GNU,Clang>:-Wall -Wextra>
        $<$<C_COMPILER_ID:MSVC>:/W4>
)

# If info-panel.c includes <gtk/gtk.h>, it MUST get GTK include dirs.
# Linking to the imported target carries include dirs + compile defs.
target_link_libraries(cairo_shared
    PUBLIC
        PkgConfig::GTK
)

target_include_directories(cairo_shared
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# libm only on Unix-like platforms
if(UNIX)
  target_link_libraries(cairo_shared PUBLIC m)
endif()

# -----------------------------
# candinista executable
# -----------------------------
add_executable(candinista
    candinista.c
    datalogging.c
    environ.c
    interpolate.c
    panel.c
    sensor.c
    gtk-glue.c
    units.c
)

target_compile_options(candinista
    PRIVATE
        $<$<C_COMPILER_ID:GNU,Clang>:-Wall -Wextra>
        $<$<C_COMPILER_ID:MSVC>:/W4>
)

target_link_libraries(candinista
    PRIVATE
        yaml_shared
        cairo_shared
        PkgConfig::GTK
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
        yaml_shared
        cairo_shared
        PkgConfig::GTK
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
