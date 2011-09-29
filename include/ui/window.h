#ifndef __PTO_UI_WINDOW_H
#define __PTO_UI_WINDOW_H

#include <ui/ui.h>

int ui_window_init(ui_widget_t** widget);
int ui_window_destroy(ui_widget_t* widget);
int ui_window_event(ui_widget_t* widget, unsigned long event);

#endif
