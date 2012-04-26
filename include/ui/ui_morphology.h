#ifndef __CLIT_UI_MORPHOLOGY_H
#define __CLIT_UI_MORPHOLOGY_H

#include <system.h>
#include <ui/ui.h>
#include <nodes/morphology.h>

#define STRUCTURAL_ELEMENT_SIZE 7
#define ELEMENT_ARRAY_SIZE STRUCTURAL_ELEMENT_SIZE * STRUCTURAL_ELEMENT_SIZE
struct ui_morphology_s
{
	GtkWidget* dialog;
	GtkWidget* drawing_area;
	morphology_operation_t op;
	unsigned int *current_element;
	unsigned int custom_element[ELEMENT_ARRAY_SIZE];
	GtkWidget* combobox;
	guint cb_handler;
};
typedef struct ui_morphology_s ui_morphology_t;

sys_result_t
ui_morphology_add_ui_string(GtkUIManager* ui_manager);

void
ui_morphology_add_action_entries(
		GtkActionGroup* action_group,
		GtkWindow *parent);

#endif
