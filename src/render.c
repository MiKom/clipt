#include <config.h>
#include <system.h>
#include <render.h>

void init_glx(void)
{
    glXChooseFBConfig =
        (PFNGLXCHOOSEFBCONFIGPROC)
        glXGetProcAddress((GLubyte*)"glXChooseFBConfig");
    glXGetVisualFromFBConfig =
        (PFNGLXGETVISUALFROMFBCONFIGPROC)
        glXGetProcAddress((GLubyte*)"glXGetVisualFromFBConfig");
    glXCreateContextAttribsARB =
        (PFNGLXCREATECONTEXTATTRIBSARBPROC)
        glXGetProcAddress((GLubyte*)"glXCreateContextAttribsARB");
}

sys_result_t
render_context_init(Window xwindow, GLXContext* out_ctx){

        init_glx();
        static int fb_attribs[] = {
                GLX_RENDER_TYPE, GLX_RGBA_BIT,
                GLX_X_RENDERABLE, True,
                GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
                GLX_DOUBLEBUFFER, False,
                GLX_RED_SIZE, 8,
                GLX_BLUE_SIZE, 8,
                GLX_GREEN_SIZE, 8,
                0
        };

        static GLint gl_attribs[] = {
                GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
                GLX_CONTEXT_MINOR_VERSION_ARB, 0,
                GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
                0
        };

        XVisualInfo* visual_info;

        Display* disp = XOpenDisplay(NULL);

        int num_configs = 0;
        GLXFBConfig* fb_configs;
        GLXContext ctx;

        fb_configs  = glXChooseFBConfig(disp, DefaultScreen(disp), fb_attribs, &num_configs);
        visual_info = glXGetVisualFromFBConfig(disp, fb_configs[0]);

        ctx = glXCreateContextAttribsARB(disp, fb_configs[0], 0, True, gl_attribs);
        glXMakeCurrent(disp, xwindow, ctx);

        XFlush(disp);
        XFree(fb_configs);
        XFree(visual_info);

        glewInit();

        *out_ctx = ctx;

        return CLIT_OK;
}

sys_result_t
render_context_free(GLXContext* ctx){

        return CLIT_OK;
}
