#include "config.h"
#include "system.h"
#include "ui/ui.h"
#include "ui/ui_morphology.h"

//Defining common structural elements
static unsigned int
horizontal_line[] = {
	0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0,
};

static unsigned int
vertical_line[] = {
	0, 0, 0, 1, 0, 0, 0,
	0, 0, 0, 1, 0, 0, 0,
	0, 0, 0, 1, 0, 0, 0,
	0, 0, 0, 1, 0, 0, 0,
	0, 0, 0, 1, 0, 0, 0,
	0, 0, 0, 1, 0, 0, 0,
	0, 0, 0, 1, 0, 0, 0,
};

static unsigned int
cross[] = {
	0, 0, 0, 1, 0, 0, 0,
	0, 0, 0, 1, 0, 0, 0,
	0, 0, 0, 1, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 1, 0, 0, 0,
	0, 0, 0, 1, 0, 0, 0,
	0, 0, 0, 1, 0, 0, 0,
};

static void
ui_morphology_show_dialog(GtkWidget *widget, gpointer data);

static ui_morphology_t*
ui_morphology_dialog_new(GtkWidget *parent);

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
	ui_morphology_t* obj = ui_morphology_dialog_new((GtkWidget*) data);

	gtk_widget_show_all(obj->dialog);
	gint response = gtk_dialog_run(obj->dialog);
	if(response == GTK_RESPONSE_ACCEPT) {

	} else {

	}
	gtk_widget_destroy(obj->dialog);
	free(obj);
}


static ui_morphology_t*
ui_morphology_dialog_new(GtkWidget *parent)
{
	ui_morphology_t *ret = (ui_morphology_t*) malloc(sizeof(ui_morphology_t));

	GtkWindow* dialog = gtk_dialog_new();
	ret->dialog = dialog;

	gtk_window_set_title(dialog, "Mathematical Morphology");
	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(parent));
	gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
	gtk_window_set_modal(GTK_WINDOW(dialog), FALSE);
	gtk_dialog_add_buttons(GTK_DIALOG(dialog),
			       "Cancel",
			       GTK_RESPONSE_CANCEL,
			       "Apply",
			       GTK_RESPONSE_APPLY,
			       NULL);

	GtkWidget *label = gtk_label_new("Operation");
	GtkWidget *box = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

	gtk_box_pack_start(GTK_BOX(box), label, FALSE, TRUE, 0);

	GtkWidget *combobox = gtk_combo_box_text_new();
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox), "erode", "Erode");
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox), "dilate", "Dilate");
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox), "open", "Open");
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox), "close", "Close");
	gtk_combo_box_set_active(GTK_COMBO_BOX(combobox), 0);
	gtk_box_pack_start(GTK_BOX(box), GTK_WIDGET(combobox), FALSE, TRUE, 0);

	label = gtk_label_new("Structural element");  //reusing variable
	gtk_widget_set_margin_top(label, 15);
	gtk_box_pack_start(GTK_BOX(box), label, FALSE, TRUE, 0);

	return ret;
}
