#include <config.h>
#include <image.h>
#include <system.h>
#include <core.h>
#include <image.h>
#include <render.h>
#include <device.h>
#include <plugin.h>

#include <ui/window.h>

// SUBSYSTEM: ui
static sys_result_t
core_ui_start(ui_widget_t** ui)
{
	*ui = ui_widget_init(NULL, CLIPT_NAME_STRING, 800, 600);
	if(ui_window_init(ui) != CLIPT_OK) {
		g_critical("Subsystem [GUI]: Failed");
		return CLIPT_ERROR;
	}

	ui_window_event(*ui, UI_EVENT_SHOW);
	g_message("Subsystem [GUI]: Started");
	return CLIPT_OK;
}

static sys_result_t
core_ui_stop(ui_widget_t* ui)
{
	ui_window_destroy(ui);
	g_message("Subsystem [GUI]: Stopped");
	return CLIPT_OK;
}

// SUBSYSTEM: render
static GLXContext render_context;

static sys_result_t
core_render_start(void)
{
	Window x_window;
	ui_widget_t* ui_drawable = ui_window_getdrawable();
	ui_widget_getnative(ui_drawable, &x_window);
	render_context_init(x_window, &render_context);
	ui_window_setglcontext(render_context);

	g_message("Subsystem [RENDER]: Started");
	return CLIPT_OK;
}

static sys_result_t
core_render_stop(void)
{
	render_context_free(ui_window_getglcontext());
	g_message("Subsystem [RENDER]: Stopped");
	return CLIPT_OK;
}

// SUBSYSTEM: device
static sys_result_t
core_device_start(void)
{
	device_context_t* device_context = sys_get_state()->context;
	if(device_create(device_context) != DEVICE_OK)
		return CLIPT_ERROR;
	g_message("Subsystem [DEVICE]: OpenCL device initialized: %s", device_context->devname);
	g_message("Subsystem [DEVICE]: Started");
	return CLIPT_OK;
}

static sys_result_t
core_device_stop(void)
{
	sys_clear_buffers();
	device_destroy(sys_get_state()->context);
	g_message("Subsystem [DEVICE]: Stopped");
	return CLIPT_OK;
}

// SUBSYSTEM: plugin
static sys_result_t
core_plugin_start(void)
{
	plugin_load_all();
	g_message("Subsystem [PLUGIN]: Started");
	return CLIPT_OK;
}

static sys_result_t
core_plugin_stop(void)
{
	plugin_unload_all();
	g_message("Subsystem [PLUGIN]: Stopped");
	return CLIPT_OK;
}

// Main
int core_main(void)
{
	sys_config_t* sys_config = sys_get_config();
	ui_widget_t*  ui;

	if(core_ui_start(&ui) != CLIPT_OK)
		return CLIPT_ERROR;

	gtk_main();
	//TODO: should it be here?
	sys_clear_buffers();
	core_ui_stop(ui);
	return CLIPT_OK;
}

device_buffer_t buf;

int core_subsystem_start(void)
{
	if(core_render_start() != CLIPT_OK)
		return CLIPT_ERROR;
	if(core_device_start() != CLIPT_OK)
		return CLIPT_ERROR;
	if(core_plugin_start() != CLIPT_OK)
		return CLIPT_ERROR;

	return CLIPT_OK;
}

int core_subsystem_stop(void)
{
	core_plugin_stop();
	core_device_stop();
	core_render_stop();
	return CLIPT_OK;
}
