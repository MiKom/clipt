#ifndef __CLIT_UI_CONVOLUTIONS_H
#define __CLIT_UI_CONVOLUTIONS_H

#include <system.h>
#include <ui/ui.h>

sys_result_t
ui_convolutions_add_ui_string(GtkUIManager *ui_manager);

void
ui_convolutions_add_action_entries(GtkActionGroup *action_group, GtkWindow *parent);

#endif
