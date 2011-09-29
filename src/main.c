#include <stdio.h>

#include <config.h>
#include <core.h>
#include <ui/window.h>

int main(int argc, char** argv)
{
	ui_widget_t* ui_window;

	gtk_init(&argc, &argv);
	
	ui_window = ui_widget_init(NULL, "Obrazator", 400, 300);
	if(ui_window_init(&ui_window) != PTO_OK) {
		fprintf(stderr, "%s: Unable to initialize GUI", argv[0]);
		return 1;
	}
	
	ui_window_event(ui_window, UI_EVENT_SHOW);
	gtk_main();
	ui_window_destroy(ui_window);
	return 0;
}
