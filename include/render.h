#ifndef __CLIT_RENDER_H
#define __CLIT_RENDER_H

#include <X11/Xlib.h>

sys_result_t render_context_init(Window xwindow);
sys_result_t render_context_free(void);

#endif