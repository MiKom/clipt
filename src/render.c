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
        XWindowAttributes attr;
        device_buffer_t* buffer;
        size_t buffer_w, buffer_h;
        int pos_x, pos_y;
        
        XGetWindowAttributes(disp, xwindow, &attr);
        
        glClearColor(0.35f, 0.35f, 0.35f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
        glDrawBuffer(GL_BACK);

        buffer = sys_get_active_buffer();
        if(buffer) {
                device_buffer_getprop(buffer, &buffer_w, &buffer_h, NULL);
                pos_x = attr.width - buffer_w;
                pos_y = attr.height - buffer_h;
                if(pos_x < 0) pos_x = 0;
                if(pos_y < 0) pos_y = 0;
                
                glWindowPos2f(pos_x*0.5f, pos_y*0.5f);
                device_buffer_draw(buffer);
        }

        glFinish();
	glXSwapBuffers(disp, xwindow);

        return CLIT_OK;
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
render_buffer_create(size_t width, size_t height, size_t channels,
                     render_buffer_t* buffer)
{
        if(width == 0 || height == 0 || channels == 0)
                return CLIT_EINVALID;

        size_t mem_size = width * height * channels * sizeof(float);
        
        glGenBuffers(1, &buffer->gl_object);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buffer->gl_object);
        glBufferData(GL_PIXEL_UNPACK_BUFFER, mem_size, NULL, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

        buffer->width    = width;
        buffer->height   = height;
        buffer->channels = channels;
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
        glDrawPixels(buffer->width, buffer->height, GL_RGB, GL_FLOAT, NULL);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}

float*
render_buffer_map(render_buffer_t* buffer, sys_access_t access)
{
        static GLenum gl_access[] = { GL_READ_ONLY, GL_WRITE_ONLY, GL_READ_WRITE };
        
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buffer->gl_object);
        buffer->hostptr = (float*)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, gl_access[access]);
        //glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        return buffer->hostptr;
}

void
render_buffer_unmap(render_buffer_t* buffer)
{
        //glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buffer->gl_object);
        glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        buffer->hostptr = NULL;
}

sys_result_t
render_buffer_copy(render_buffer_t* src, render_buffer_t* dst,
                   size_t offset, size_t size)
{
        GLenum gl_error;
        
        glBindBuffer(GL_COPY_READ_BUFFER, src->gl_object);
        glBindBuffer(GL_COPY_WRITE_BUFFER, dst->gl_object);
        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER,
                            offset, offset, size);
        gl_error = glGetError();
        glBindBuffer(GL_COPY_READ_BUFFER, 0);
        glBindBuffer(GL_COPY_WRITE_BUFFER, 0);

        if(gl_error != 0)
                return CLIT_ERROR;
        return CLIT_OK;                
}
