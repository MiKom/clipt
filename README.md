CLIPT - OpenCL Image Processing Toolkit
=======================================

This file describes CLIPT. It's a simple image processing toolkit
with GPU computation in mind.

About
-----

### Overview

This project aims to be a simple appliation with various implementations of 
image processing algorithms implemented mainly on GPU in one place. It's
GUI is made with GTK3, with image rendering through OpenGL and operations
in OpenCL (more on that later).

### History

This project was started by Miłosz Kosobucki and Michał Siejak as a 
programming assignment for *Image processing* class that took place in winter
semester in 2011/2012 acadaemic year at *Adam Mickiewicz University of Poznań*
under supervision of Mr Wojciech Kowalewski Ph.D..

### Authors

As mentioned before:

 - Miłosz Kosobucki - implemented OpenGL context in GTK, most of gui, plugin
 api implementation, file loading plugin api, ppm loading/saving plugin, all
 operations except convolutions.
 - Michał Siejak - designed and implemented OpenCL/OpenGL buffer sharing
 system, plugin api, main program structure (subsystem initialization, 
 configuration handling, main loop), convolutions, OpenCL context.

Configuration
-------------

Program looks for .cliptrc.ini in folder specified in `XDG_CONFIG_HOME`.
If this variable doesn't exist, program tries to find it in current directory.

If it's still not found, following values are assumed:

	plugin-dir=/usr/lib/clipt
	clprog-dir=/usr/lib/clipt/cl

`plugin-dir` should point to a directory with clipt plugins

`clprog-dir` should point to a directory with OpenCL kernels

### Config file format

`.cliptrc.ini` is a simple .ini file with variables defined in `[clipt]`
category:

	[clipt]
	plugin-dir=/usr/lib/clipt
	clprog-dir=/usr/lib/clipt/cl

Architecture
------------

### UI

User interface is implemented using GTK+3. However, the canvas on which
the images are drawn is an OpenGL window. Since GTK3 doesn't have any
support for embedding OpenGL, it's done by a nasty hack, that takes
native X11 window handler and tries to establish GLX context in that window.

UI routines are place in `ui/` subfolders of `include/` and `src/` directories.

### GPGPU

Most algorithms are implemented using OpenCL. Kernel files are read and
compiled on runtime. Directory path for kernels is specified by configuration
file (more on that later).

### OpenCL <--> OpenGL interop

To avoid sending big images back and forth through PCI Express bus this project
uses buffer sharing between OpenCL and OpenGL introduced in OpenCL 1.1.

Internal image format is 32 bits per channel RGB stored as floats. This
fact establishes 1GB of VRAM as a reasonable minimum when working with large
(4k x 4k) images, since two copies of the image must be stored in GPU memory.

When user previews some filter/operation and adjusts parameters in real time,
the data is not leaving the GPU. This makes interaction with program more
smooth. Only when the user hits *Apply* on some effect window, the result of
the GPU computation is sent to RAM.

Interoperability routines and buffer sharing mechanisms are implemented in
device.c and render.c.

### Plugins

CLIPT implements rather crude plugin mechanism. It tries to load dynamic
libraries from specified folder and initialize them.

Plugin API currently works only for file loading/saving plugins and is used by
Portable Anymap file plugin.

Features
--------

### Implemented operations

On GPU:

 - simple curve manipulation (brigtness, gamma, contrast)
 - histogram (showing, stretching, equalization)
 - mathematical morphology
 - convolution (custom and some predefined masks)
 - Thresholding
 - Otsu binarization

### Supported file types

 - Portable Anymap - reading and writing. Both ASCII and raw version of PPM
 are supported
