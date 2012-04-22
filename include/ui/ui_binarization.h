#ifndef __CLIT_UI_BINARIZATION_H
#define __CLIT_UI_BINARIZATION_H

#include <system.h>

sys_result_t
ui_binarization_add_ui_string(GtkUIManager *ui_manager);

void
ui_binarization_add_action_entries(
		GtkActionGroup *action_group,
		GtkWindow *parent);

#endif
