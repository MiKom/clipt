#ifndef __CLIPT_UI_COLORSPACES_H
#define __CLIPT_UI_COLORSPACES_H

#include<system.h>

sys_result_t
ui_colorspaces_add_ui_string(GtkUIManager *ui_manager);

void
ui_colorspaces_add_action_entries(
		GtkActionGroup *action_group,
		GtkWindow *parent);
#endif
