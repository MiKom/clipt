#ifndef __CLIPT_UI_CONVOLUTIONS_H
#define __CLIPT_UI_CONVOLUTIONS_H

#include <system.h>
#include <ui/ui.h>

sys_result_t
ui_convolutions_add_ui_string(GtkUIManager *ui_manager);

void
ui_convolutions_add_action_entries(GtkActionGroup *action_group, GtkWindow *parent);

struct ui_convolutions_s
{
        GtkWidget* dialog;
        GtkTextBuffer* buffer;
	GtkSpinButton* bias;
	GtkSpinButton* divisor;
};
typedef struct ui_convolutions_s ui_convolutions_t;

#endif
