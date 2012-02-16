#ifndef __CLIT_RENDER_H
#define __CLIT_RENDER_H

#include <X11/Xlib.h>
#include <system.h>

struct render_buffer_s {
        GLuint gl_object;
        void*  hostptr;
        size_t width;
        size_t height;
        size_t bpp;
};
typedef struct render_buffer_s render_buffer_t;

sys_result_t render_context_init(Window xwindow, GLXContext* out_ctx);
sys_result_t render_context_draw(Window xwindow, GLXContext* ctx);
sys_result_t render_context_free(GLXContext ctx);

sys_result_t render_buffer_create(size_t width, size_t height, size_t bpp,
                                  render_buffer_t* buffer);
sys_result_t render_buffer_destroy(render_buffer_t* buffer);
void render_buffer_draw(render_buffer_t* buffer);

#endif
