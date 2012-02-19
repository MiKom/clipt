#include <config.h>
#include <image.h>
#include <system.h>
#include <plugin.h>
#include <render.h>
#include <device.h>

#include <stdio.h>

static device_context_t device_context;
static Display* disp;
static float color;

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
render_context_init(Window xwindow, GLXContext* out_ctx)
{

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
                GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
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
        int cur_width;
        int cur_height;
        XWindowAttributes attr;

        XGetWindowAttributes(disp, xwindow, &attr);
        cur_width	= attr.width;
        cur_height	= attr.height;

        g_debug("Width: %d, Height: %d", cur_width, cur_height);

        glClearColor(0.0, color, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	color += 0.02;
	if(color > 1.0f) {
		color = 0.0f;
        }


        // DEBUG
        glGetError();
        
        static int once = 0;
        static GLuint buf;
        static unsigned char* ptr;
	glWindowPos2f((cur_width - 320.0f)/2.0f, (cur_height - 240.0f)/2.0f );
        if(!once) {
                once = 1;

                ptr = malloc(320*240*3);

                glGenBuffers(1, &buf);
                glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buf);
                glBufferData(GL_PIXEL_UNPACK_BUFFER, 320*240*3, NULL, GL_DYNAMIC_DRAW);
                //ptr = glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
                memset(ptr, 255, 320*240*3);
                //glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
                glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        }

        glBindBuffer(GL_PIXEL_PACK_BUFFER, buf);
        glDrawBuffer(GL_BACK);
        glDrawPixels(320, 240, GL_RGB, GL_UNSIGNED_BYTE, ptr);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
        //printf("%s\n", gluErrorString(glGetError()));

        glFinish();
        fflush(stdout);
        // END DEBUG
        
        //device_buffer_draw(sys_get_active_buffer());
	glXSwapBuffers(disp, xwindow);
        printf("----------\n");
}


sys_result_t
render_context_free(GLXContext ctx)
{
	device_destroy(&device_context);
	glXDestroyContext(disp, ctx);
	XCloseDisplay(disp);
	return CLIT_OK;
}

sys_result_t
render_buffer_create(size_t width, size_t height, size_t bpp,
                     render_buffer_t* buffer)
{
        if(width == 0 || height == 0 || bpp == 0)
                return CLIT_EINVALID;
        
        glGenBuffers(1, &buffer->gl_object);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, buffer->gl_object);
        glBufferData(GL_PIXEL_PACK_BUFFER, width*height*bpp, NULL, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

        buffer->width   = width;
        buffer->height  = height;
        buffer->bpp     = bpp;
        return CLIT_OK;
}

sys_result_t
render_buffer_destroy(render_buffer_t* buffer)
{
        glDeleteBuffers(1, &buffer->gl_object);
        buffer->gl_object = 0;
        return CLIT_OK;
}

void
render_buffer_draw(render_buffer_t* buffer)
{
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buffer->gl_object);
        glDrawPixels(buffer->width, buffer->height, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}

