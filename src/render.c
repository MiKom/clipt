#include <config.h>
#include <system.h>
#include <image.h>
#include <plugin.h>
#include <render.h>
#include <device.h>

#include <stdio.h>

static device_context_t device_context;

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

static Display* disp;
static float color;

sys_result_t
render_context_init(Window xwindow, GLXContext* out_ctx){

        init_glx();
        static int fb_attribs[] = {
                GLX_X_RENDERABLE, True,
                GLX_RENDER_TYPE, GLX_RGBA_BIT,
                GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
                GLX_DOUBLEBUFFER, False,
                GLX_RED_SIZE, 8,
                GLX_BLUE_SIZE, 8,
                GLX_GREEN_SIZE, 8,
                None
        };

        static GLint gl_attribs[] = {
                GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
                GLX_CONTEXT_MINOR_VERSION_ARB, 0,
                GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
                0
        };

        XVisualInfo* visual_info;

        disp = XOpenDisplay(NULL);

        int num_configs = 0;
        GLXFBConfig* fb_configs;
        GLXContext ctx;

        fb_configs  = glXChooseFBConfig(disp, DefaultScreen(disp), fb_attribs, &num_configs);
        visual_info = glXGetVisualFromFBConfig(disp, fb_configs[0]);

        int (*oldHandler)(Display*, XErrorEvent*) =
              XSetErrorHandler(NULL);

        ctx = glXCreateContextAttribsARB(disp, fb_configs[0], 0, True, gl_attribs);
        if(!glXMakeCurrent(disp, xwindow, ctx) ){
            fprintf(stderr, "Couldn't bind OpenGL context to X window\n");
        }

        XFlush(disp);
        XFree(fb_configs);
        XFree(visual_info);

        GLenum glew_error = glewInit();
        if(glew_error != GLEW_OK) {
            fprintf(stderr, "glew error: %s\n",glewGetErrorString(glew_error));
        }

        *out_ctx = ctx;

        glClearColor(0.0,1.0,0.0,1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        glXSwapBuffers(disp,xwindow);
        XSetErrorHandler(oldHandler);
        color = 0.0f;

        device_create(&device_context);
        return CLIT_OK;
}

sys_result_t
render_context_draw(Window xwindow, GLXContext* ctx)
{
        glClearColor(0.0, color, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        color += 0.02;
        if(color > 1.0f) {
                color = 0.0f;
        }
        glXSwapBuffers(disp, xwindow);
}


sys_result_t
render_context_free(GLXContext* ctx){
        device_destroy(&device_context);
        glXDestroyContext(disp, *ctx);
        XCloseDisplay(disp);
        return CLIT_OK;
}
