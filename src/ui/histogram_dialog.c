#include <config.h>
#include <system.h>
#include <ui/histogram_dialog.h>
#include <ui/window.h>

static void
ui_show_histogram_action_cb(GtkWidget* widget, gpointer data);

static void
ui_stretch_histogram_action_cb(GtkWidget* widget, gpointer data);

static void
ui_equalize_histogram_action_cb(GtkWidget* widget, gpointer data);

static void
ui_histogram_redraw_cb(GtkWidget* widget, gpointer data);

static gboolean
ui_histogram_close_cb(GtkWidget* widget, GdkEvent* event, gpointer data);

static void
ui_histogram_surface_draw_cb (GtkWidget* widget, cairo_t* cr, gpointer data);

static void
ui_histogram_combobox_changed_cb (GtkWidget* widget, gpointer data);

static char* ui_def =
"<ui>"
"  <menubar name='MainMenu'>"
"    <menu name='ColourMenu' action='ColourMenuAction'>"
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
	G_CALLBACK(ui_show_histogram_action_cb)},

	{"EqualizeAction", NULL,
	 "Equalize", NULL,
	 "Equalize histogram of the image",
	 G_CALLBACK(ui_equalize_histogram_action_cb)},

	{"StretchAction", NULL,
	 "Stretch", NULL,
	 "Stretch histogram of the image",
	 G_CALLBACK(ui_stretch_histogram_action_cb)},
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
ui_histogram_add_action_entries(GtkActionGroup* action_group, GtkWindow *parent)
{
	gtk_action_group_add_actions(action_group, actions, n_actions, parent);
}

static void ui_show_histogram_action_cb(GtkWidget* widget, gpointer data)
{
	ui_histogram_t *hist_obj = malloc(sizeof(ui_histogram_t));

	GtkWidget* window;
	GtkWidget* vbox;
	GtkWidget* combobox;
	GtkWidget* drawing_area;

	// setting up window
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "Histogram");
	gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
	g_signal_connect(window, "delete-event",
			 G_CALLBACK(ui_histogram_close_cb), hist_obj);
	hist_obj->cb_handler = ui_window_add_image_changed_cb(
				G_CALLBACK(ui_histogram_redraw_cb), hist_obj);
	hist_obj->window = window;

	//setting up vbox
	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), vbox);

	//setting up combobox
	combobox = gtk_combo_box_text_new();
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox), "lum", "Luminance");
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox), "red", "Red channel");
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox), "green", "Green channel");
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox), "blue", "Blue channel");
	gtk_combo_box_set_active(GTK_COMBO_BOX(combobox), 0);
	g_signal_connect(GTK_WIDGET(combobox), "changed",
			 G_CALLBACK(ui_histogram_combobox_changed_cb), hist_obj);
	gtk_box_pack_start(GTK_BOX(vbox), combobox, FALSE, FALSE, 0);
	hist_obj->combobox = combobox;


	//setting up drawing area
	drawing_area = gtk_drawing_area_new();
	gtk_widget_set_size_request(GTK_WIDGET(drawing_area), 264, 256);
	gtk_box_pack_start(GTK_BOX(vbox), drawing_area, FALSE, FALSE, 5);
	g_signal_connect(GTK_WIDGET(drawing_area), "draw",
			 G_CALLBACK(ui_histogram_surface_draw_cb), hist_obj);
	hist_obj->drawing_area = drawing_area;

	gtk_widget_show_all(window);

}

static void ui_histogram_redraw_cb(GtkWidget* widget, gpointer data)
{
        ui_histogram_t *obj = (ui_histogram_t*) data;
        g_debug("Recalculating histogram");
}

static void
ui_histogram_surface_draw_cb (GtkWidget* widget, cairo_t* cr, gpointer data)
{
	ui_histogram_t *obj = (ui_histogram_t*) data;

//FOR TESTING
	unsigned int histogram[256];
	srand(1);
	int i;
	for(i=0; i<256; i++) {
		histogram[i] = rand() % 256;
	}
	histogram[0] = histogram[255] = 255;
//END

	unsigned int maxval = histogram[0];
	for(i=1; i<256; i++) {
		if(histogram[i] > maxval) {
			maxval = histogram[i];
		}
	}
	gchar* text = gtk_combo_box_get_active_id(GTK_COMBO_BOX_TEXT(obj->combobox));
	if(g_strcmp0(text, "lum") == 0) {
		cairo_set_source_rgb (cr, 0, 0, 0);
	} else if(g_strcmp0(text, "red") == 0) {
		cairo_set_source_rgb (cr, 0.8, 0, 0);
	} else if(g_strcmp0(text, "green") == 0) {
		cairo_set_source_rgb (cr, 0, 0.8, 0);
	} else if(g_strcmp0(text, "blue") == 0) {
		cairo_set_source_rgb (cr, 0, 0, 0.8);
	}

	for(i=0; i<256; i++) {
		cairo_move_to(cr, (double)(i+5), 256.0);
		double line_h = (double)histogram[i] / (double)maxval * 256.0;
		cairo_line_to(cr, (double)(i+5), 256.0 - line_h);
	}
	cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);
	cairo_stroke(cr);
	cairo_set_antialias(cr, CAIRO_ANTIALIAS_DEFAULT);
}

void
ui_histogram_combobox_changed_cb (GtkWidget* widget, gpointer data)
{
	ui_histogram_t *obj = (ui_histogram_t*) data;
	gtk_widget_queue_draw(obj->drawing_area);
}

gboolean
ui_histogram_close_cb(GtkWidget* widget, GdkEvent* event, gpointer data)
{
	ui_histogram_t *obj = (ui_histogram_t*) data;
	ui_window_remove_image_changed_handler(obj->cb_handler);
	free(obj);
	return FALSE;
}

static void
ui_equalize_histogram_action_cb(GtkWidget* widget, gpointer data)
{
	g_debug("Equalize histogram action");
}

static void
ui_stretch_histogram_action_cb(GtkWidget* widget, gpointer data)
{
	g_debug("Stretch histogram action");
}
