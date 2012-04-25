CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(CLIT)
SET(CMAKE_C_FLAGS "-ggdb -fPIC -pipe")
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/plugins)

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/CMakeModules)

ADD_DEFINITIONS("-Wall -Wextra -Wpointer-arith -Wcast-qual
  -Wno-unused-parameter -fstrict-aliasing")

FIND_PACKAGE(PkgConfig)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)


INCLUDE_DIRECTORIES( ${GTK3_INCLUDE_DIRS} ${CLIT_SOURCE_DIR}/include)

FIND_PACKAGE(OpenGL)
INCLUDE_DIRECTORIES( ${OPENGL_INCLUDE_DIR} )

FIND_PACKAGE(GLEW)
INCLUDE_DIRECTORIES( ${GLEW_INCLUDE_PATH} )

FIND_PACKAGE(OpenCL)
INCLUDE_DIRECTORIES( ${OPENCL_INCLUDE_DIRS} )

ADD_SUBDIRECTORY(plugins)

FILE(COPY clprogs DESTINATION ${PROJECT_BINARY_DIR})
SET( clit_SOURCES
	src/main.c
	src/core.c
	src/system.c
	src/render.c
	src/plugin.c
	src/device.c
	src/io.c
	src/image.c

	src/ui/common.c
	src/ui/window.c
	src/ui/ui_histogram.c
	src/ui/ui_curves.c
	src/ui/ui_convolutions.c
	src/ui/ui_binarization.c
	src/ui/ui_colorspaces.c
	src/ui/ui_morphology.c

	src/nodes/histogram.c
	src/nodes/curves.c
	src/nodes/binarization.c
	src/nodes/colorspaces.c
	src/nodes/morphology.c
)

SET( clit_CLPROGS
	clprogs/lut.cl
	clprogs/gamma.cl
	clprogs/brightness.cl
	clprogs/histogram.cl
	clprogs/binarization.cl
	clprogs/colorspaces.cl
	clprogs/morphology.cl
)

SET( clit_HEADERS
	include/clit.h
	include/config.h
	include/core.h
	include/image.h
	include/plugin.h
	include/render.h
	include/system.h
	include/device.h
	include/io.h

	include/ui/ui.h
	include/ui/window.h
	include/ui/ui_histogram.h
	include/ui/ui_curves.h
	include/ui/ui_convolutions.h
	include/ui/ui_binarization.h
	include/ui/ui_colorspaces.h
	include/ui/ui_morphology.h

	include/nodes/histogram.h
	include/nodes/curves.h
	include/nodes/binarization.h
	include/nodes/colorspaces.h
	include/nodes/morphology.h
)

SET(COMMON_LIBRARIES m dl)

ADD_EXECUTABLE(clit ${clit_SOURCES} ${clit_HEADERS} ${clit_CLPROGS})
TARGET_LINK_LIBRARIES(clit ${COMMON_LIBRARIES} ${GTK3_LIBRARIES} ${GLEW_LIBRARY} ${OPENGL_LIBRARIES} ${OPENCL_LIBRARIES} )
