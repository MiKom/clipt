#ifndef __CLIT_UI_WINDOW_H
#define __CLIT_UI_WINDOW_H

#include <system.h>
#include <ui/ui.h>

sys_result_t ui_window_init(ui_widget_t** widget);
int ui_window_destroy(ui_widget_t* widget);
int ui_window_event(ui_widget_t* widget, unsigned long event);

ui_widget_t*
ui_window_getdrawable(void);

GLXContext
ui_window_getglcontext(void);

void
ui_window_setglcontext(GLXContext ctx);

/**
 * Forces redrawing of image from the current draw buffer. Use for previews
 * with sliders etc.
 **/
void
ui_window_force_redraw(void);

/**
 * \return handle id of the connected callback, use for removal
 **/
gulong
ui_window_add_image_changed_cb(GCallback cb, gpointer data);

void
ui_window_remove_image_changed_handler(gulong handler_id);

#endif
