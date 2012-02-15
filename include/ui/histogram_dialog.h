#ifndef __CLIT_UI_HISTOGRAM_DIALOG_H
#define __CLIT_UI_HISTOGRAM_DIALOG_H

#include <ui/ui.h>

struct ui_histogram_s
{
	ui_widget_t* window;
	GtkWidget* drawing_area;
	GtkWidget* combobox;
	guint cb_handler;
};
typedef struct ui_histogram_s ui_histogram_t;

char* ui_histogram_get_ui_string();

GtkActionEntry*
ui_histogram_get_action_entry();

#endif
