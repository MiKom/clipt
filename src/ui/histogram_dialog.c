#include <config.h>
#include <system.h>
#include <ui/histogram_dialog.h>
#include <ui/window.h>

static void ui_histogram_menu_action_cb(GtkWidget* widget, gpointer data);
static void ui_histogram_redraw_cb(GtkWidget* widget, gpointer data);
static void ui_histogram_close_cb(GtkWidget* widget, gpointer data);

static char* ui_def =
"<ui>"
"  <menubar name='MainMenu'>"
"    <menu name='ColorMenu' action='ColorMenuAction'>"
"      <menuitem action='HistogramAction'/>"
"    </menu>"
"  </menubar>"
"</ui>";

static GtkActionEntry entries[] = {
	{"HistogramAction", "color-histogram",
	"_Histogram", NULL,
	"Open new histogram window",
	G_CALLBACK(ui_histogram_menu_action_cb)}
};

char* ui_histogram_get_ui_string()
{
	return ui_def;
}

GtkActionEntry*
ui_histogram_get_action_entry()
{
	return entries;
}

static void ui_histogram_menu_action_cb(GtkWidget* widget, gpointer data)
{
	ui_widget_t *ui_hist_window = ui_widget_init(NULL, "Histogram window", 256, 256);
	ui_hist_window->widget = gtk_window_new(GTK_WINDOW_TOPLEVEL);


	ui_histogram_t *hist_obj = malloc(sizeof(ui_histogram_t));

	hist_obj->window = ui_hist_window;
	hist_obj->drawing_area = gtk_drawing_area_new();

	gtk_widget_show(ui_hist_window->widget);

	hist_obj->cb_handler = ui_window_add_image_changed_cb(
				G_CALLBACK(ui_histogram_redraw_cb), hist_obj);
	g_debug("handler: %d", hist_obj->cb_handler);

	g_signal_connect(ui_hist_window->widget, "delete-event",
			 G_CALLBACK(ui_histogram_close_cb), hist_obj);
}

static void ui_histogram_redraw_cb(GtkWidget* widget, gpointer data)
{
}

static void ui_histogram_close_cb(GtkWidget* widget, gpointer data)
{
	ui_histogram_t *obj = (ui_histogram_t*) data;
	ui_window_remove_image_changed_handler(obj->cb_handler);
}
