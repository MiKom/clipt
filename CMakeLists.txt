CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(CLIT)
SET(CMAKE_C_FLAGS "-ggdb -fPIC -pipe")
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/plugins)

ADD_DEFINITIONS("-Wall -Wextra -Wpointer-arith -Wcast-qual
  -Wno-unused-parameter -fstrict-aliasing")

FIND_PACKAGE(PkgConfig)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

INCLUDE_DIRECTORIES( ${GTK3_INCLUDE_DIRS} ${CLIT_SOURCE_DIR}/include)
ADD_SUBDIRECTORY(plugins)

ADD_EXECUTABLE(clit src/main.c src/core.c src/system.c src/ui/common.c src/ui/window.c)
TARGET_LINK_LIBRARIES(clit ${GTK3_LIBRARIES})
