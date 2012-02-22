#ifndef __CLIT_RENDER_H
#define __CLIT_RENDER_H

#include <X11/Xlib.h>
#include <system.h>

struct render_buffer_s {
        GLuint gl_object;
        float* hostptr;
        size_t width;
        size_t height;
        size_t channels;
};
typedef struct render_buffer_s render_buffer_t;

sys_result_t render_context_init(Window xwindow, GLXContext* out_ctx);
sys_result_t render_context_draw(Window xwindow, GLXContext* ctx);
sys_result_t render_context_free(GLXContext ctx);

sys_result_t render_buffer_create(size_t width, size_t height, size_t channels,
                                  render_buffer_t* buffer);
sys_result_t render_buffer_destroy(render_buffer_t* buffer);

void   render_buffer_draw(render_buffer_t* buffer);
float* render_buffer_map(render_buffer_t* buffer, sys_access_t access);
void   render_buffer_unmap(render_buffer_t* buffer);
sys_result_t render_buffer_copy(render_buffer_t* src, render_buffer_t* dst,
                                size_t offset, size_t size);

#endif
