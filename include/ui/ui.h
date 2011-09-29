#ifndef __PTO_UI_H
#define __PTO_UI_H

#include <gtk/gtk.h>
#include <glib/gi18n.h>

#define UI_EVENT_IDLE   0x00
#define UI_EVENT_SHOW   0x01
#define UI_EVENT_HIDE   0x02

struct ui_widget_s
{
	GtkWidget* widget;
	
	char title[256];
	int  width;
	int  height;
};
typedef struct ui_widget_s ui_widget_t;

ui_widget_t* ui_widget_init(ui_widget_t* widget, const char* title, int w, int h);
ui_widget_t* ui_widget_defaults(ui_widget_t* widget, const char* title, int w, int h);
void         ui_widget_free(ui_widget_t* widget);

#endif
