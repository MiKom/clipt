#include <config.h>
#include <system.h>
#include <core.h>

#include <ui/window.h>

int core_main(void)
{
	sys_config_t* sys_config = sys_get_config();
	
	// TODO: Stack based subsystem startup/shutdown
	
	ui_widget_t* ui_window = ui_widget_init(NULL, CLIT_NAME_STRING, 400, 300);
	if(ui_window_init(&ui_window) != CLIT_OK) {
		g_critical("Unable to initialize GUI");
		return CLIT_ERROR;
	}

	ui_window_event(ui_window, UI_EVENT_SHOW);
	gtk_main();
	ui_window_destroy(ui_window);
	
	return CLIT_OK;
}
