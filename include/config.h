#ifndef __CLIPT_CONFIG_H
#define __CLIPT_CONFIG_H

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include <glib.h>
#include <GL/glew.h>
#include <GL/glxew.h>

#if defined (__APPLE__) || defined(MACOSX)
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#include <CL/cl_gl.h>
#endif

#define CLIPT_VERSION_MAJOR 0
#define CLIPT_VERSION_MINOR 1

#define CLIPT_NAME_STRING    "CLIPT: OpenCL Image Processing Toolkit"
#define CLIPT_VERSION_STRING "0.1.0"
#define CLIPT_PROGRAM_NAME   "clipt"

#define CLIPT_DEFAULT_CONFIG "cliptrc.ini"
#define CLIPT_DEFAULT_SYMBOL "clipt_plugin_info"

#endif
