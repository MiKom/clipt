#ifndef __CLIT_UI_WINDOW_H
#define __CLIT_UI_WINDOW_H

#include <system.h>
#include <ui/ui.h>

sys_result_t ui_window_init(ui_widget_t** widget);
int ui_window_destroy(ui_widget_t* widget);
int ui_window_event(ui_widget_t* widget, unsigned long event);

#endif
