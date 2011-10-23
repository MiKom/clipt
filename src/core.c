#include <config.h>
#include <system.h>
#include <core.h>

#include <ui/window.h>

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

int core_main(void)
{
	sys_config_t* sys_config = sys_get_config();
    ui_widget_t* ui;
	
	if(core_ui_start(&ui) != CLIT_OK)
        return CLIT_ERROR;

	gtk_main();

    core_ui_stop(ui);
	return CLIT_OK;
}
