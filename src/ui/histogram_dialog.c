#include <config.h>
#include <system.h>
#include <ui/histogram_dialog.h>
#include <ui/window.h>

static void
ui_histogram_menu_action_cb(GtkWidget* widget, gpointer data);

static void
ui_histogram_redraw_cb(GtkWidget* widget, gpointer data);

static gboolean
ui_histogram_close_cb(GtkWidget* widget, GdkEvent* event, gpointer data);

static char* ui_def =
"<ui>"
"  <menubar name='MainMenu'>"
"    <menu name='ColorMenu' action='ColorMenuAction'>"
"      <menu name='HistogramMenu' action='HistogramMenuAction'>"
"        <menuitem action='ShowHistogramAction'/>"
"        <menuitem action='EqualizeAction'/>"
"        <menuitem action='StretchAction'/>"
"      </menu>"
"    </menu>"
"  </menubar>"
"</ui>";

static GtkActionEntry actions[] = {
	{"HistogramMenuAction", NULL, "_Histogram"},

	{"ShowHistogramAction", "color-histogram",
	"Show histogram", NULL,
	"Open new histogram window",
	G_CALLBACK(ui_histogram_menu_action_cb)},

	{"EqualizeAction", NULL,
	 "Equalize", NULL,
	 "Equalize histogram of the image",
	 NULL},

	{"StretchAction", NULL,
	 "Stretch", NULL,
	 "Stretch histogram of the image",
	 NULL}
};
static guint n_actions = G_N_ELEMENTS(actions);

sys_result_t
ui_histogram_add_ui_string(GtkUIManager* ui_manager)
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
ui_histogram_add_action_entries(GtkActionGroup* action_group)
{
	gtk_action_group_add_actions(action_group, actions, n_actions, NULL);
}

static void ui_histogram_menu_action_cb(GtkWidget* widget, gpointer data)
{
	ui_histogram_t *hist_obj = malloc(sizeof(ui_histogram_t));

	// setting up window
	GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(window, "Histogram");
	gtk_window_set_default_size(window, 256, 256);
	g_signal_connect(window, "delete-event",
			 G_CALLBACK(ui_histogram_close_cb), hist_obj);

	hist_obj->cb_handler = ui_window_add_image_changed_cb(
				G_CALLBACK(ui_histogram_redraw_cb), hist_obj);
	hist_obj->window = window;

	//setting up drawing area
	GtkWidget* drawing_area = gtk_drawing_area_new();
	hist_obj->drawing_area = drawing_area;

	gtk_widget_show_all(window);

}

static void ui_histogram_redraw_cb(GtkWidget* widget, gpointer data)
{
	ui_histogram_t *obj = (ui_histogram_t*) data;
}

static gboolean
ui_histogram_close_cb(GtkWidget* widget, GdkEvent* event, gpointer data)
{
	ui_histogram_t *obj = (ui_histogram_t*) data;
	ui_window_remove_image_changed_handler(obj->cb_handler);
	free(obj);
	return FALSE;
}
