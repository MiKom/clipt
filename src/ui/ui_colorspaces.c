#include"config.h"
#include"system.h"
#include"ui/ui.h"
#include"ui/ui_colorspaces.h"
#include"ui/window.h"
#include"nodes/colorspaces.h"

static void
ui_colorspaces_desaturize(GtkWidget *widget, gpointer data);

static char ui_def[] =
"<ui>"
"  <menubar name='MainMenu'>"
"    <menu name='ColourMenu' action='ColourMenuAction'>"
"      <menuitem action='DesaturateAction' />"
"    </menu>"
"  </menubar>"
"</ui>";

static GtkActionEntry actions[] = {
	{ "DesaturateAction", NULL, "Desaturate", NULL,
	  "Desaturate image",
	  G_CALLBACK(ui_colorspaces_desaturize)},
};
static guint n_actions = G_N_ELEMENTS(actions);

sys_result_t
ui_colorspaces_add_ui_string(GtkUIManager *ui_manager)
{
	GError *error = NULL;
	gtk_ui_manager_add_ui_from_string(GTK_UI_MANAGER(ui_manager), ui_def,
					  -1, &error);
	if(error) {
		g_error_free(error);
		return CLIPT_ERROR;
	}
	return CLIPT_OK;
}

void
ui_colorspaces_add_action_entries(
		GtkActionGroup *action_group,
		GtkWindow *parent)
{
	gtk_action_group_add_actions(GTK_ACTION_GROUP(action_group), actions,
				    n_actions, parent);
}

static void
ui_colorspaces_desaturize(GtkWidget *widget, gpointer data)
{
	colorspaces_init();
	colorspaces_desaturate(sys_get_current_buffer(), sys_get_draw_buffer());
	sys_commit_buffer(sys_get_draw_buffer());
	ui_window_force_redraw();
}
