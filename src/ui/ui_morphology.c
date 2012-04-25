#include "config.h"
#include "system.h"
#include "ui/ui.h"
#include "ui/ui_morphology.h"

static void
ui_morphology_show_dialog(GtkWidget *widget, gpointer data);

static char* ui_def =
"<ui>"
"  <menubar name='MainMenu'>"
"    <menu name='FiltersMenu' action='FiltersMenuAction'>"
"      <menuitem action='MorphologyAction' />"
"    </menu>"
"  </menubar>"
"</ui>";

static GtkActionEntry actions[] = {
	{"MorphologyAction", "color-curves",
	 "Morphology", NULL,
	 "Appply mathematical morphology operations",
	 G_CALLBACK(ui_morphology_show_dialog)}
};
static guint n_actions = G_N_ELEMENTS(actions);

sys_result_t
ui_morphology_add_ui_string(GtkUIManager* ui_manager)
{
	GError* error = NULL;
	gtk_ui_manager_add_ui_from_string(ui_manager, ui_def, -1, &error);
	if(error) {
		g_error_free(error);
		return CLIT_ERROR;
	}
	return CLIT_OK;
}

void
ui_morphology_add_action_entries(GtkActionGroup* action_group, GtkWindow *parent)
{
	gtk_action_group_add_actions(action_group, actions, n_actions, parent);
}

static void
ui_morphology_show_dialog(GtkWidget *widget, gpointer data)
{

}
