#include <config.h>
#include <system.h>
#include <core.h>
#include <render.h>
#include <device.h>

#include <ui/window.h>

// SUBSYSTEM: ui
static sys_result_t
core_ui_start(ui_widget_t** ui)
{
        *ui = ui_widget_init(NULL, CLIT_NAME_STRING, 400, 300);
        if(ui_window_init(ui) != CLIT_OK) {
                g_critical("Subsystem [GUI]: Failed");
                return CLIT_ERROR;
        }

        ui_window_event(*ui, UI_EVENT_SHOW);
        g_message("Subsystem [GUI]: Started");
        return CLIT_OK;
}

static sys_result_t
core_ui_stop(ui_widget_t* ui)
{
        ui_window_destroy(ui);
        g_message("Subsystem [GUI]: Stopped");
        return CLIT_OK;
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
        return CLIT_OK;
}

static sys_result_t
core_render_stop(void)
{
        render_context_free(ui_window_getglcontext());
        g_message("Subsystem [RENDER]: Stopped");
        return CLIT_OK;
}

// SUBSYSTEM: device
static device_context_t device_context;

static sys_result_t
core_device_start(void)
{
        if(device_create(&device_context) != DEVICE_OK)
                return CLIT_ERROR;
        g_message("Subsystem [DEVICE]: OpenCL device initialized: %s", device_context.devname);
        g_message("Subsystem [DEVICE]: Started");
        return CLIT_OK;
}

static sys_result_t
core_device_stop(void)
{
        device_destroy(&device_context);
        g_message("Subsystem [DEVICE]: Stopped");
        return CLIT_OK;
}

// SUBSYSTEM: plugin
static sys_result_t
core_plugin_start(void)
{
        g_message("Subsystem [PLUGIN]: Started");
        return CLIT_OK;
}

static sys_result_t
core_plugin_stop(void)
{
        g_message("Subsystem [PLUGIN]: Stopped");
        return CLIT_OK;
}

// Main
int core_main(void)
{
        sys_config_t* sys_config = sys_get_config();
        ui_widget_t*  ui;

        if(core_ui_start(&ui) != CLIT_OK)
                return CLIT_ERROR;

        gtk_main();
        core_ui_stop(ui);
        return CLIT_OK;
}

int core_subsystem_start(void)
{
        if(core_render_start() != CLIT_OK)
                return CLIT_ERROR;
        if(core_device_start() != CLIT_OK)
                return CLIT_ERROR;
        if(core_plugin_start() != CLIT_OK)
                return CLIT_ERROR;
        
        return CLIT_OK;
}

int core_subsystem_stop(void)
{
        core_plugin_stop();
        core_device_stop();
        core_render_stop();
        return CLIT_OK;
}
