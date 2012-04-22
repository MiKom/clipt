#include"config.h"
#include"system.h"
#include"ui/ui.h"
#include"ui/window.h"
#include"ui/ui_binarization.h"
#include"nodes/binarization.h"


static void
ui_binarization_threshold_dialog(GtkWidget *widget, gpointer data);

static void
ui_threshold_scale_cb(GtkWidget *widget, gpointer data);

static char ui_def[] =
"<ui>"
"  <menubar name='MainMenu'>"
"    <menu name='FiltersMenu' action='FiltersMenuAction'>"
"      <menu name='BinarizationMenu' action='BinarizationMenuAction'>"
"        <menuitem action='ThresholdAction' />"
"        <menuitem action='OtsuAction' />"
"      </menu>"
"    </menu>"
"  </menubar>"
"</ui>";

static GtkActionEntry actions[] = {
	{ "BinarizationMenuAction", NULL, "Binarization"},

	{ "ThresholdAction", NULL, "Threshold", NULL,
	  "Binarize by hand with scale",
	  G_CALLBACK(ui_binarization_threshold_dialog)},
	{ "OtsuAction", NULL, "Otsu", NULL,
	  "Binarize with Otsu algorithm", NULL }
};
static guint n_actions = G_N_ELEMENTS(actions);

sys_result_t
ui_binarization_add_ui_string(GtkUIManager *ui_manager)
{
	GError *error = NULL;
	gtk_ui_manager_add_ui_from_string(GTK_UI_MANAGER(ui_manager), ui_def,
					  -1, &error);
	if(error) {
		g_error_free(error);
		return CLIT_ERROR;
	}
	return CLIT_OK;
}

void
ui_binarization_add_action_entries(
		GtkActionGroup *action_group,
		GtkWindow *parent)
{
	gtk_action_group_add_actions(GTK_ACTION_GROUP(action_group), actions,
				    n_actions, parent);
}

static void
ui_binarization_threshold_dialog(GtkWidget *widget, gpointer data)
{
	GtkWidget *dialog;

	binarization_init();

	dialog = gtk_dialog_new();
	gtk_window_set_title(GTK_WINDOW(dialog), "Thresholding");
	GtkWindow *parent = (GtkWindow*) data;
	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(parent));
	gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
	gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
	gtk_dialog_add_buttons(GTK_DIALOG(dialog),
			       "Cancel",
			       GTK_RESPONSE_CANCEL,
			       "Apply",
			       GTK_RESPONSE_APPLY,
			       NULL);


	GtkWidget *scale;
	scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL,
					  0.0, 255.0, 1.0);
	g_signal_connect(G_OBJECT(scale), "value-changed",
			  G_CALLBACK(ui_threshold_scale_cb), NULL);
	gtk_range_set_value(GTK_RANGE(scale), 127.0);
	GtkWidget *box = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	gtk_box_pack_start(GTK_BOX(box), GTK_WIDGET(scale), FALSE, FALSE, 5);

	gtk_widget_show_all(GTK_WIDGET(box));
	gint response;
	response = gtk_dialog_run(GTK_DIALOG(dialog));

	if( response == GTK_RESPONSE_APPLY ) {
		sys_commit_buffer(sys_get_draw_buffer());
	} else if ( response == GTK_RESPONSE_CANCEL ) {
		sys_draw_current_buffer();
	}
	gtk_widget_destroy(GTK_WIDGET(dialog));
	ui_window_force_redraw();
}

static void
ui_threshold_scale_cb(GtkWidget *widget, gpointer data)
{
	gdouble threshold = gtk_range_get_value(GTK_RANGE(widget));

	threshold_binarization(sys_get_current_buffer(), sys_get_draw_buffer(),
			       (unsigned int) threshold);
	ui_window_force_redraw();
}
