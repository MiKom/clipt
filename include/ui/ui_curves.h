#ifndef __CLIPT_UI_CURVES_H
#define __CLIPT_UI_CURVES_H

#include <system.h>
#include <ui/ui.h>

struct ui_curves_dialog_s
{
        GtkWidget *dialog;
        GtkWidget *combobox;
        GtkWidget *scale;
        GtkWidget *drawing_area;
        int disp_lut[256];
};
typedef struct ui_curves_dialog_s ui_curves_dialog_t;

sys_result_t
ui_curves_add_ui_string(GtkUIManager* ui_manager);

void
ui_curves_add_action_entries(GtkActionGroup* action_group, GtkWindow *parent);

#endif
