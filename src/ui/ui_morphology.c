#include "config.h"
#include "system.h"
#include "ui/ui.h"
#include "ui/window.h"
#include "ui/ui_morphology.h"
#include "nodes/morphology.h"

//size of the pixel used by drawing areas that draw structural elements,
//effectively it's a magnification factor
#define PIXEL_SIZE 6

static const char ERODE_ID[] = "erode";
static const char DILATE_ID[] = "dilate";
static const char OPEN_ID[] = "open";
static const char CLOSE_ID[] = "close";

//Defining common structural elements
static unsigned int
hline[] = {
	0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0,
};

static unsigned int
vline[] = {
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

static void ui_morphology_do(ui_morphology_t* obj);

/**
  This function takes pointer to array of STRUCTURAL_ELEMENT_SIZE*STRUCTURAL_ELEMENT_SIZE
  unsigned int elements in data, that contain zeroes or ones, and draws grid
  of black and white pixels, according to this array. This grid is magniefied,
  so single pixel is PIXEL_SIZE x PIXEL_SIZE in size.
  */
static void
ui_morphology_draw_structural_element(
		GtkWidget *widget,
		cairo_t *cr,
		gpointer data);

static gboolean
ui_morphology_button_press(
		GtkWidget *widget,
		GdkEventButton *event,
		gpointer data);

struct ui_morphology_radio_data_s
{
	ui_morphology_t *object;
	unsigned int *element;
};
typedef struct ui_morphology_radio_data_s ui_morphology_radio_data_t;

static void
ui_morphology_radio_toggle(GtkWidget *widget, gpointer data);

static void
ui_morphology_combobox_changed(GtkWidget *widget, gpointer data);

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
	morphology_init();
	morphology_allocate_temp(sys_get_current_buffer());
	ui_morphology_t* obj = ui_morphology_dialog_new((GtkWidget*) data);

	gtk_widget_show_all(obj->dialog);
	ui_morphology_do(obj);


	gint response = gtk_dialog_run(obj->dialog);
	if( response == GTK_RESPONSE_APPLY ) {
		sys_commit_buffer(sys_get_draw_buffer());
	} else if ( response == GTK_RESPONSE_CANCEL ) {
		sys_draw_current_buffer();
	}
	gtk_widget_destroy(obj->dialog);

	morphology_deallocate_temp();
	free(obj);
	ui_window_force_redraw();
}

static ui_morphology_t*
ui_morphology_dialog_new(GtkWidget *parent)
{
	ui_morphology_t *ret = (ui_morphology_t*) malloc(sizeof(ui_morphology_t));

	GtkWindow* dialog = gtk_dialog_new();
	ret->dialog = dialog;
	memset(ret->custom_element, 0, sizeof(ret->custom_element));

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
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox), ERODE_ID, "Erode");
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox), DILATE_ID, "Dilate");
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox), OPEN_ID, "Open");
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox), CLOSE_ID, "Close");
	gtk_combo_box_set_active(GTK_COMBO_BOX(combobox), 0);
	gtk_box_pack_start(GTK_BOX(box), GTK_WIDGET(combobox), FALSE, TRUE, 0);
	g_signal_connect(G_OBJECT(combobox), "changed",
			 G_CALLBACK(ui_morphology_combobox_changed), ret);
	gtk_combo_box_set_active_id(GTK_COMBO_BOX(combobox), ERODE_ID);
	ret->op = MORPHOLOGY_ERODE;

	label = gtk_label_new("Structural element");  //reusing variable
	gtk_widget_set_margin_top(label, 15);
	gtk_box_pack_start(GTK_BOX(box), label, FALSE, TRUE, 0);

	GtkWidget *horizontal_radio = gtk_radio_button_new_with_label(NULL, "Horizontal line");
	g_object_set_data(G_OBJECT(horizontal_radio), "element", hline);
	g_signal_connect(G_OBJECT(horizontal_radio), "toggled",
			 G_CALLBACK(ui_morphology_radio_toggle), ret);

	GtkWidget *vertical_radio = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(horizontal_radio),
										"Vertical line");
	g_object_set_data(G_OBJECT(vertical_radio), "element", vline);
	g_signal_connect(G_OBJECT(vertical_radio), "toggled",
			 G_CALLBACK(ui_morphology_radio_toggle), ret);

	GtkWidget *cross_radio = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(vertical_radio),
									     "Cross");
	g_object_set_data(G_OBJECT(cross_radio), "element", cross);
	g_signal_connect(G_OBJECT(cross_radio), "toggled",
			 G_CALLBACK(ui_morphology_radio_toggle), ret);

	GtkWidget *custom_radio = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(cross_radio),
									      "Custom");
	g_object_set_data(G_OBJECT(custom_radio), "element", ret->custom_element);
	g_signal_connect(G_OBJECT(custom_radio), "toggled",
			 G_CALLBACK(ui_morphology_radio_toggle), ret);

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(horizontal_radio), TRUE);
	ret->current_element = hline;

	GtkWidget *grid = gtk_grid_new();
	gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
	gtk_grid_set_column_spacing(GTK_GRID(grid), 10);

	gtk_grid_attach(GTK_GRID(grid), horizontal_radio, 0, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), vertical_radio, 0, 1, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), cross_radio, 0, 2, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), custom_radio, 0, 3, 1, 1);

	gtk_box_pack_start(GTK_BOX(box), grid, TRUE, TRUE, 0);

	GtkWidget *horizontal_draw = gtk_drawing_area_new();
	gtk_grid_attach_next_to(GTK_GRID(grid), horizontal_draw, horizontal_radio, GTK_POS_RIGHT,1, 1);
	gtk_widget_set_size_request(GTK_WIDGET(horizontal_draw),
				    STRUCTURAL_ELEMENT_SIZE*PIXEL_SIZE,
				    STRUCTURAL_ELEMENT_SIZE*PIXEL_SIZE);
	g_signal_connect(GTK_WIDGET(horizontal_draw), "draw",
			 G_CALLBACK(ui_morphology_draw_structural_element),
			 (gpointer) hline);

	GtkWidget *vertical_draw = gtk_drawing_area_new();
	gtk_grid_attach_next_to(GTK_GRID(grid), vertical_draw, vertical_radio, GTK_POS_RIGHT,1, 1);
	gtk_widget_set_size_request(GTK_WIDGET(vertical_draw),
				    STRUCTURAL_ELEMENT_SIZE*PIXEL_SIZE,
				    STRUCTURAL_ELEMENT_SIZE*PIXEL_SIZE);
	g_signal_connect(GTK_WIDGET(vertical_draw), "draw",
			 G_CALLBACK(ui_morphology_draw_structural_element),
			 (gpointer) vline);

	GtkWidget *cross_draw = gtk_drawing_area_new();
	gtk_grid_attach_next_to(GTK_GRID(grid), cross_draw, cross_radio, GTK_POS_RIGHT,1, 1);
	gtk_widget_set_size_request(GTK_WIDGET(cross_draw),
				    STRUCTURAL_ELEMENT_SIZE*PIXEL_SIZE,
				    STRUCTURAL_ELEMENT_SIZE*PIXEL_SIZE);
	g_signal_connect(GTK_WIDGET(cross_draw), "draw",
			 G_CALLBACK(ui_morphology_draw_structural_element),
			 (gpointer) cross);

	GtkWidget *custom_draw = gtk_drawing_area_new();
	gtk_grid_attach_next_to(GTK_GRID(grid), custom_draw, custom_radio, GTK_POS_RIGHT,1, 1);
	gtk_widget_set_size_request(GTK_WIDGET(custom_draw),
				    STRUCTURAL_ELEMENT_SIZE*PIXEL_SIZE,
				    STRUCTURAL_ELEMENT_SIZE*PIXEL_SIZE);
	g_signal_connect(GTK_WIDGET(custom_draw), "draw",
			 G_CALLBACK(ui_morphology_draw_structural_element),
			 (gpointer) ret->custom_element);
	g_signal_connect(custom_draw, "button-press-event",
			 G_CALLBACK(ui_morphology_button_press), ret);
	gtk_widget_set_events(custom_draw, gtk_widget_get_events(custom_draw)
			      | GDK_BUTTON_PRESS_MASK);
	return ret;
}

static void
ui_morphology_draw_structural_element(
		GtkWidget *widget,
		cairo_t *cr,
		gpointer data)
{
	unsigned int *element = (unsigned int*) data;
	cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
	cairo_paint(cr);
	cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
	int i,j;
	for(i=0; i<STRUCTURAL_ELEMENT_SIZE; i++) {
		for(j=0; j<STRUCTURAL_ELEMENT_SIZE; j++) {
			if(element[i*STRUCTURAL_ELEMENT_SIZE + j] == 0 ) {
				continue;
			} else {
				double x = PIXEL_SIZE * j;
				double y = PIXEL_SIZE * i;
				cairo_rectangle(cr,x, y, PIXEL_SIZE, PIXEL_SIZE);
			}
		}
	}
	cairo_fill(cr);
}

static gboolean
ui_morphology_button_press(
		GtkWidget *widget,
		GdkEventButton *event,
		gpointer data)
{
	if(event->type == GDK_BUTTON_PRESS) {
		ui_morphology_t* obj = (ui_morphology_t*)(data);
		unsigned int *element = obj->custom_element;
		int x = (int) (event->x / PIXEL_SIZE);
		int y = (int) (event->y / PIXEL_SIZE);
		int idx = y*STRUCTURAL_ELEMENT_SIZE + x;
		element[idx] = (element[idx] + 1) % 2;
		gtk_widget_queue_draw(widget);
		if(obj->current_element == obj->custom_element) {
			ui_morphology_do(obj);
		}
		return TRUE;
	} else {
		return FALSE;
	}
}

static void
ui_morphology_combobox_changed(GtkWidget *widget, gpointer data)
{
	gchar *id = gtk_combo_box_get_active_id(GTK_COMBO_BOX(widget));
	ui_morphology_t *obj = (ui_morphology_t*) data;
	if(g_strcmp0(id, ERODE_ID) == 0) {
		obj->op = MORPHOLOGY_ERODE;
	} else if(g_strcmp0(id, DILATE_ID) == 0) {
		obj->op = MORPHOLOGY_DILATE;
	} else if(g_strcmp0(id, OPEN_ID) == 0) {
		obj->op = MORPHOLOGY_OPEN;
	} else if(g_strcmp0(id, CLOSE_ID) == 0) {
		obj->op = MORPHOLOGY_CLOSE;
	}
	ui_morphology_do(obj);
}

void
ui_morphology_radio_toggle(GtkWidget *widget, gpointer data)
{
	ui_morphology_t *obj = (ui_morphology_t*) data;
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
		obj->current_element = (unsigned int*) g_object_get_data(widget,
									 "element");
		ui_morphology_do(obj);
	}
}

static void ui_morphology_do(ui_morphology_t* obj)
{
	morphology_apply(sys_get_current_buffer(), sys_get_draw_buffer(),
			 obj->op, obj->current_element, STRUCTURAL_ELEMENT_SIZE);
	ui_window_force_redraw();
}
