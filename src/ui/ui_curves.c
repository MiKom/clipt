#include <config.h>

#include <ui/ui_curves.h>
#include <ui/window.h>

#include <nodes/curves.h>

#define LEFT_PIXEL_OFFSET 2.0

static void
ui_curves_show_dialog(GtkWidget* widget, gpointer data);

static void
ui_curves_scale_cb(GtkWidget* widget, gpointer data);

static void
ui_curves_combobox_changed_cb(GtkWidget* widget, gpointer data);

static void
ui_curves_drawing_area_redraw_cb(GtkWidget* widget, cairo_t *cr, gpointer data);

static char* ui_def =
"<ui>"
"  <menubar name='MainMenu'>"
"    <menu name='ColourMenu' action='ColourMenuAction'>"
"      <menuitem action='CurvesAction' />"
"    </menu>"
"  </menubar>"
"</ui>";

static GtkActionEntry actions[] = {
	{"CurvesAction", "color-curves",
	 "Curves", NULL,
	 "Manipulate curves of colors",
	 G_CALLBACK(ui_curves_show_dialog)}
};
static guint n_actions = G_N_ELEMENTS(actions);

sys_result_t
ui_curves_add_ui_string(GtkUIManager* ui_manager)
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
ui_curves_add_action_entries(GtkActionGroup* action_group, GtkWindow *parent)
{
	gtk_action_group_add_actions(action_group, actions, n_actions, parent);
}

static void
ui_curves_show_dialog(GtkWidget* widget, gpointer data)
{
	ui_curves_dialog_t *obj = malloc(sizeof(ui_curves_dialog_t));

	GtkWidget *dialog;
	GtkWidget *combobox;
	GtkWidget *scale;
	GtkWidget *drawing_area;

	GtkWidget *box;
	GtkWidget *label;

	GtkWindow* parent = (GtkWindow*) data;

	gint response;

	curves_init();

	curves_get_neutral_lut8(obj->disp_lut);

	dialog = gtk_dialog_new();
	gtk_window_set_title(GTK_WINDOW(dialog), "Curves");
	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(parent));
	gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
	gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
	gtk_dialog_add_buttons(GTK_DIALOG(dialog),
			       "Cancel",
			       GTK_RESPONSE_CANCEL,
			       "Apply",
			       GTK_RESPONSE_APPLY,
			       NULL);
	obj->dialog = dialog;
	box = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

	//setting up drawing area
	drawing_area = gtk_drawing_area_new();
	gtk_widget_set_size_request(GTK_WIDGET(drawing_area),
				    256 + 2*LEFT_PIXEL_OFFSET, 258);
	gtk_box_pack_start(GTK_BOX(box), drawing_area, TRUE, FALSE, 0);
	g_signal_connect(G_OBJECT(drawing_area), "draw",
			 G_CALLBACK(ui_curves_drawing_area_redraw_cb), obj);
	obj->drawing_area = drawing_area;

	//setting up combobox
	label = gtk_label_new("Operation");
	gtk_box_pack_start(GTK_BOX(box), GTK_WIDGET(label), FALSE, FALSE, 0);
	combobox = gtk_combo_box_text_new();
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox),
				  "brightness", "Adjust brightness");
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox),
				  "gamma", "Adjust gamma");
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox),
				  "contrast", "Adjust contrast");
	gtk_combo_box_set_active(GTK_COMBO_BOX(combobox), 0);
	g_signal_connect(G_OBJECT(combobox), "changed",
			 G_CALLBACK(ui_curves_combobox_changed_cb), obj);
	gtk_box_pack_start(GTK_BOX(box), combobox, TRUE, TRUE, 0);
	obj->combobox = combobox;

	//setting up the scale
	scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL,
					 -255.0, 255.0, 1.0);
	gtk_range_set_value(GTK_RANGE(scale), 0.0);
	g_signal_connect(G_OBJECT(scale), "value-changed",
			 G_CALLBACK(ui_curves_scale_cb), obj);
	gtk_box_pack_start(GTK_BOX(box), scale, FALSE, FALSE, 5);
	obj->scale = scale;

	gtk_widget_show_all(GTK_WIDGET(box));
	response = gtk_dialog_run(GTK_DIALOG(dialog));

	if( response == GTK_RESPONSE_APPLY ) {
		sys_commit_buffer(sys_get_draw_buffer());
	} else if ( response == GTK_RESPONSE_CANCEL ) {
		sys_draw_current_buffer();
	}
	gtk_widget_destroy(GTK_WIDGET(dialog));
	ui_window_force_redraw();

	free(obj);
}

static void
ui_curves_scale_cb(GtkWidget* widget, gpointer data)
{
	ui_curves_dialog_t *obj = (ui_curves_dialog_t*) data;

	gdouble value = gtk_range_get_value(GTK_RANGE(obj->scale));

	const gchar* box_id = gtk_combo_box_get_active_id(
				GTK_COMBO_BOX(obj->combobox));
	if( g_strcmp0(box_id,"brightness") == 0) {
		curves_get_brightness_lut8((int) value, obj->disp_lut);
	} else if( g_strcmp0(box_id, "gamma") == 0) {
		curves_get_gamma_lut8(value, obj->disp_lut);
	} else if( g_strcmp0(box_id, "contrast") == 0 ) {
		curves_get_contrast_lut8((int) value, obj->disp_lut);
	}

	curves_apply_lut8(sys_get_current_buffer(), sys_get_draw_buffer(), obj->disp_lut);
	gtk_widget_queue_draw(obj->drawing_area);

	ui_window_force_redraw();
}

static void
ui_curves_combobox_changed_cb(GtkWidget* widget, gpointer data)
{
	ui_curves_dialog_t *obj = (ui_curves_dialog_t*) data;

	const gchar* box_id = gtk_combo_box_get_active_id(
				GTK_COMBO_BOX(obj->combobox));
	if( g_strcmp0(box_id,"brightness") == 0) {
		gtk_range_set_range(GTK_RANGE(obj->scale), -255.0f, 255.0f);
		gtk_range_set_increments(GTK_RANGE(obj->scale), 1.0f, 10.0f);
		gtk_range_set_value(GTK_RANGE(obj->scale), 0.0f);
		gtk_scale_set_digits(GTK_SCALE(obj->scale), 0);
	} else if( g_strcmp0(box_id, "gamma") == 0) {
		gtk_range_set_range(GTK_RANGE(obj->scale), 0.01f, 10.0f);
		gtk_range_set_increments(GTK_RANGE(obj->scale), 0.01f, 0.5f);
		gtk_range_set_value(GTK_RANGE(obj->scale), 1.0f);
		gtk_scale_set_digits(GTK_SCALE(obj->scale), 2);
	} else if( g_strcmp0(box_id, "contrast") == 0 ) {
		gtk_range_set_range(GTK_RANGE(obj->scale), -255.0f, 255.0f);
		gtk_range_set_increments(GTK_RANGE(obj->scale), 1.0f, 10.0f);
		gtk_range_set_value(GTK_RANGE(obj->scale), 0.0f);
		gtk_scale_set_digits(GTK_SCALE(obj->scale), 0);
	}

}

static void
ui_curves_drawing_area_redraw_cb(GtkWidget* widget, cairo_t *cr, gpointer data)
{
	ui_curves_dialog_t *obj = (ui_curves_dialog_t*) data;
	int* lut = obj->disp_lut;

//FOR TESTING
	unsigned int histogram[256];
	srand(1);
	int i;
	for(i=0; i<256; i++) {
		histogram[i] = rand() % 256;
	}
//END

	unsigned int maxval = histogram[0];
	for(i=1; i<256; i++) {
		if(histogram[i] > maxval) {
			maxval = histogram[i];
		}
	}

	cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
	cairo_paint(cr);

	double offset = LEFT_PIXEL_OFFSET;

	cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
	for(i=0; i<256; i++) {
		cairo_move_to(cr, (double)i + offset, 256.0);
		double line_h = (double)histogram[i] / (double)maxval * 256.0;
		cairo_line_to(cr, (double)i + offset, 256.0 - line_h);
	}
	cairo_stroke(cr);
	cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
	cairo_move_to(cr, offset, 256.0 - (double) lut[0]);
	for(i=0; i<256; i++) {
		cairo_line_to(cr, (double) i + offset, 256.0 - (double) lut[i]);
	}
	cairo_stroke(cr);
}
